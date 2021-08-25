/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Pietro Iglio <iglio@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdesktopfile.h"

#include "kauthorized.h"
#include "kconfig_core_log_settings.h"
#include "kconfig_p.h"
#include "kconfiggroup.h"
#include "kconfigini_p.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

#include <algorithm>

class KDesktopFilePrivate : public KConfigPrivate
{
public:
    KDesktopFilePrivate(QStandardPaths::StandardLocation resourceType, const QString &fileName);
    KConfigGroup desktopGroup;
};

KDesktopFilePrivate::KDesktopFilePrivate(QStandardPaths::StandardLocation resourceType, const QString &fileName)
    : KConfigPrivate(KConfig::NoGlobals, resourceType)
{
    mBackend = new KConfigIniBackend();
    bDynamicBackend = false;
    changeFileName(fileName);
}

KDesktopFile::KDesktopFile(QStandardPaths::StandardLocation resourceType, const QString &fileName)
    : KConfig(*new KDesktopFilePrivate(resourceType, fileName))
{
    Q_D(KDesktopFile);
    reparseConfiguration();
    d->desktopGroup = KConfigGroup(this, "Desktop Entry");
}

KDesktopFile::KDesktopFile(const QString &fileName)
    : KConfig(*new KDesktopFilePrivate(QStandardPaths::ApplicationsLocation, fileName))
{
    Q_D(KDesktopFile);
    reparseConfiguration();
    d->desktopGroup = KConfigGroup(this, "Desktop Entry");
}

KDesktopFile::~KDesktopFile()
{
}

KConfigGroup KDesktopFile::desktopGroup() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup;
}

QString KDesktopFile::locateLocal(const QString &path)
{
    QString relativePath;
    QChar plus(QLatin1Char('/'));
    // Relative to config? (e.g. for autostart)
    const QStringList lstGenericConfigLocation = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    // Iterate from the last item since some items may be subfolders of others.
    for (QStringList::const_reverse_iterator constIterator = lstGenericConfigLocation.crbegin(); constIterator != lstGenericConfigLocation.crend();
         ++constIterator) {
        const QString &dir = (*constIterator);
        if (path.startsWith(dir + plus)) {
            relativePath = path.mid(dir.length() + 1);
            return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + relativePath;
        }
    }
    // Relative to xdg data dir? (much more common)
    const QStringList lstGenericDataLocation = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &dir : lstGenericDataLocation) {
        if (path.startsWith(dir + plus)) {
            relativePath = path.mid(dir.length() + 1);
        }
    }
    if (relativePath.isEmpty()) {
        // What now? The desktop file doesn't come from XDG_DATA_DIRS. Use filename only and hope for the best.
        relativePath = path.mid(path.lastIndexOf(QLatin1Char('/')) + 1);
    }
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + relativePath;
}

bool KDesktopFile::isDesktopFile(const QString &path)
{
    return path.endsWith(QLatin1String(".desktop"));
}

bool KDesktopFile::isAuthorizedDesktopFile(const QString &path)
{
    if (path.isEmpty()) {
        return false; // Empty paths are not ok.
    }

    if (QDir::isRelativePath(path)) {
        return true; // Relative paths are ok.
    }

    const QString realPath = QFileInfo(path).canonicalFilePath();
    if (realPath.isEmpty()) {
        return false; // File doesn't exist.
    }

#ifndef Q_OS_WIN
    const Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
#else
    const Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
#endif

    // Check if the .desktop file is installed as part of KDE or XDG.
    const QStringList appsDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    for (const QString &prefix : appsDirs) {
        if (QDir(prefix).exists() && realPath.startsWith(QFileInfo(prefix).canonicalFilePath(), sensitivity)) {
            return true;
        }
    }
    const QString servicesDir = QStringLiteral("kservices5/"); // KGlobal::dirs()->xdgDataRelativePath("services")
    const QStringList lstGenericDataLocation = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &xdgDataPrefix : lstGenericDataLocation) {
        if (QDir(xdgDataPrefix).exists()) {
            const QString prefix = QFileInfo(xdgDataPrefix).canonicalFilePath();
            if (realPath.startsWith(prefix + QLatin1Char('/') + servicesDir, sensitivity)) {
                return true;
            }
        }
    }
    const QString autostartDir = QStringLiteral("autostart/");
    const QStringList lstConfigPath = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    for (const QString &xdgDataPrefix : lstConfigPath) {
        if (QDir(xdgDataPrefix).exists()) {
            const QString prefix = QFileInfo(xdgDataPrefix).canonicalFilePath();
            if (realPath.startsWith(prefix + QLatin1Char('/') + autostartDir, sensitivity)) {
                return true;
            }
        }
    }

    // Forbid desktop files outside of standard locations if kiosk is set so
    if (!KAuthorized::authorize(QStringLiteral("run_desktop_files"))) {
        qCWarning(KCONFIG_CORE_LOG) << "Access to '" << path << "' denied because of 'run_desktop_files' restriction.";
        return false;
    }

    // Not otherwise permitted, so only allow if the file is executable, or if
    // owned by root (uid == 0)
    QFileInfo entryInfo(path);
    if (entryInfo.isExecutable() || entryInfo.ownerId() == 0) {
        return true;
    }

    qCInfo(KCONFIG_CORE_LOG) << "Access to '" << path << "' denied, not owned by root and executable flag not set.";
    return false;
}

QString KDesktopFile::readType() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readEntry("Type", QString());
}

QString KDesktopFile::readIcon() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readEntry("Icon", QString());
}

QString KDesktopFile::readName() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readEntry("Name", QString());
}

QString KDesktopFile::readComment() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readEntry("Comment", QString());
}

QString KDesktopFile::readGenericName() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readEntry("GenericName", QString());
}

QString KDesktopFile::readPath() const
{
    Q_D(const KDesktopFile);
    // NOT readPathEntry, it is not XDG-compliant: it performs
    // various expansions, like $HOME.  Note that the expansion
    // behaviour still happens if the "e" flag is set, maintaining
    // backwards compatibility.
    return d->desktopGroup.readEntry("Path", QString());
}

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 82)
QString KDesktopFile::readDevice() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readEntry("Dev", QString());
}
#endif

QString KDesktopFile::readUrl() const
{
    Q_D(const KDesktopFile);
    if (hasDeviceType()) {
        return d->desktopGroup.readEntry("MountPoint", QString());
    } else {
        // NOT readPathEntry (see readPath())
        QString url = d->desktopGroup.readEntry("URL", QString());
        if (!url.isEmpty() && !QDir::isRelativePath(url)) {
            // Handle absolute paths as such (i.e. we need to escape them)
            return QUrl::fromLocalFile(url).toString();
        }
        return url;
    }
}

QStringList KDesktopFile::readActions() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readXdgListEntry("Actions");
}

QStringList KDesktopFile::readMimeTypes() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readXdgListEntry("MimeType");
}

KConfigGroup KDesktopFile::actionGroup(const QString &group)
{
    return KConfigGroup(this, QLatin1String("Desktop Action ") + group);
}

const KConfigGroup KDesktopFile::actionGroup(const QString &group) const
{
    return const_cast<KDesktopFile *>(this)->actionGroup(group);
}

bool KDesktopFile::hasActionGroup(const QString &group) const
{
    return hasGroup(QString(QLatin1String("Desktop Action ") + group).toUtf8().constData());
}

bool KDesktopFile::hasLinkType() const
{
    return readType() == QLatin1String("Link");
}

bool KDesktopFile::hasApplicationType() const
{
    return readType() == QLatin1String("Application");
}

bool KDesktopFile::hasDeviceType() const
{
    return readType() == QLatin1String("FSDevice");
}

bool KDesktopFile::tryExec() const
{
    Q_D(const KDesktopFile);
    // Test for TryExec and "X-KDE-AuthorizeAction"
    // NOT readPathEntry (see readPath())
    QString te = d->desktopGroup.readEntry("TryExec", QString());

    if (!te.isEmpty()) {
        if (QStandardPaths::findExecutable(te).isEmpty()) {
            return false;
        }
    }
    const QStringList list = d->desktopGroup.readEntry("X-KDE-AuthorizeAction", QStringList());
    const auto isNotAuthorized = std::any_of(list.cbegin(), list.cend(), [](const QString &action) {
        return !KAuthorized::authorize(action.trimmed());
    });
    if (isNotAuthorized) {
        return false;
    }

    // See also KService::username()
    const bool su = d->desktopGroup.readEntry("X-KDE-SubstituteUID", false);
    if (su) {
        QString user = d->desktopGroup.readEntry("X-KDE-Username", QString());
        if (user.isEmpty()) {
            user = QString::fromLocal8Bit(qgetenv("ADMIN_ACCOUNT"));
        }
        if (user.isEmpty()) {
            user = QStringLiteral("root");
        }
        if (!KAuthorized::authorize(QLatin1String("user/") + user)) {
            return false;
        }
    }

    return true;
}

/**
 * @return the filename as passed to the constructor.
 */
// QString KDesktopFile::fileName() const { return backEnd->fileName(); }

/**
 * @return the resource type as passed to the constructor.
 */
// QString
// KDesktopFile::resource() const { return backEnd->resource(); }

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 42)
QStringList KDesktopFile::sortOrder() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readXdgListEntry("SortOrder");
}
#endif

// void KDesktopFile::virtual_hook( int id, void* data )
//{ KConfig::virtual_hook( id, data ); }

QString KDesktopFile::readDocPath() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readPathEntry("X-DocPath", QString());
}

KDesktopFile *KDesktopFile::copyTo(const QString &file) const
{
    KDesktopFile *config = new KDesktopFile(QString());
    this->KConfig::copyTo(file, config);
    //  config->setDesktopGroup();
    return config;
}

QStandardPaths::StandardLocation KDesktopFile::resource() const
{
    Q_D(const KDesktopFile);
    return d->resourceType;
}

QString KDesktopFile::fileName() const
{
    return name();
}

bool KDesktopFile::noDisplay() const
{
    Q_D(const KDesktopFile);
    if (d->desktopGroup.readEntry("NoDisplay", false)) {
        return true;
    }
    if (d->desktopGroup.hasKey("OnlyShowIn")) {
        if (!d->desktopGroup.readXdgListEntry("OnlyShowIn").contains(QLatin1String("KDE"))) {
            return true;
        }
    }
    if (d->desktopGroup.hasKey("NotShowIn")) {
        if (d->desktopGroup.readXdgListEntry("NotShowIn").contains(QLatin1String("KDE"))) {
            return true;
        }
    }
    return false;
}
