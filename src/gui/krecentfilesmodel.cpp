/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2026 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "config-kconfig.h"

#include "krecentfilesmodel.h"
#include "krecentfilesmodel_p.h"

#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QMimeDatabase>

#if KCONFIG_USE_DBUS
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#endif

#include <KConfig>
#include <KConfigGroup>

#include <memory_resource>
#include <set>

using namespace Qt::StringLiterals;

KRecentFilesModel::KRecentFilesModel(QObject *parent)
    : QAbstractListModel(parent)
    , d_ptr(new KRecentFilesModelPrivate(this))
{
}

KRecentFilesModel::~KRecentFilesModel() = default;

int KRecentFilesModel::maxItems() const
{
    Q_D(const KRecentFilesModel);
    return d->m_maxItems;
}

void KRecentFilesModel::setMaxItems(int maxItems)
{
    Q_D(KRecentFilesModel);
    // set new maxItems
    d->m_maxItems = std::max(maxItems, 0);

    // Remove all excess items, oldest (i.e. first added) first
    const int difference = static_cast<int>(d->m_recentFiles.size()) - d->m_maxItems;
    if (difference > 0) {
        auto beginIt = d->m_recentFiles.begin();
        auto endIt = d->m_recentFiles.begin() + difference;
        beginRemoveRows({}, d->m_recentFiles.size() - difference, d->m_recentFiles.size() - 1);
        d->m_recentFiles.erase(beginIt, endIt);
        endRemoveRows();
    }
}

void KRecentFilesModel::addUrl(const QUrl &url, const QString &name)
{
    addUrl(url, name, QString());
}

void KRecentFilesModel::addUrl(const QUrl &url, const QString &name, const QString &mimeTypeStr)
{
    Q_D(KRecentFilesModel);

    // ensure we never add items if we want none
    if (d->m_maxItems == 0) {
        return;
    }

    if (url.isLocalFile() && url.toLocalFile().startsWith(QDir::tempPath())) {
        return;
    }

    // Remove url if it already exists in the list
    removeUrl(url);

    // Remove oldest item if already maxItems in list
    if (static_cast<int>(d->m_recentFiles.size()) == d->m_maxItems) {
        beginRemoveRows({}, d->m_recentFiles.size() - 1, d->m_recentFiles.size() - 1);
        d->m_recentFiles.erase(d->m_recentFiles.begin());
        endRemoveRows();
    }

    const QString pathOrUrl(url.toDisplayString(QUrl::PreferLocalFile));
    const QString title = !name.isEmpty() ? name : url.fileName();
#ifdef Q_OS_WIN
    const QString file = url.isLocalFile() ? QDir::toNativeSeparators(pathOrUrl) : pathOrUrl;
#else
    const QString file = pathOrUrl;
#endif

#if KCONFIG_USE_DBUS
    static bool isKdeSession = qgetenv("XDG_CURRENT_DESKTOP") == "KDE";
    if (isKdeSession) {
        const QDBusConnection bus = QDBusConnection::sessionBus();
        if (bus.isConnected()
            && bus.interface()->isServiceRegistered(QStringLiteral("org.kde.ActivityManager"))
            // skip files in hidden directories
            && !url.path().contains(QStringLiteral("/."))) {
            const static QString activityService = QStringLiteral("org.kde.ActivityManager");
            const static QString activityResources = QStringLiteral("/ActivityManager/Resources");
            const static QString activityResouceInferface = QStringLiteral("org.kde.ActivityManager.Resources");

            QString mimeTypeName = mimeTypeStr;
            if (mimeTypeName.isEmpty()) {
                mimeTypeName = QMimeDatabase().mimeTypeForFile(url.path(), QMimeDatabase::MatchExtension).name();
            }

            const auto urlString = url.toString(QUrl::PreferLocalFile);
            QDBusMessage message =
                QDBusMessage::createMethodCall(activityService, activityResources, activityResouceInferface, QStringLiteral("RegisterResourceEvent"));
            message.setArguments({qApp->desktopFileName(), uint(0) /* WinId */, urlString, uint(0) /* eventType Accessed */});
            bus.asyncCall(message);

            message = QDBusMessage::createMethodCall(activityService, activityResources, activityResouceInferface, QStringLiteral("RegisterResourceMimetype"));
            message.setArguments({urlString, mimeTypeName});
            bus.asyncCall(message);

            message = QDBusMessage::createMethodCall(activityService, activityResources, activityResouceInferface, QStringLiteral("RegisterResourceTitle"));
            message.setArguments({urlString, url.fileName()});
            bus.asyncCall(message);
        }
    }
#endif

    beginInsertRows(QModelIndex(), 0, 0);
    d->m_recentFiles.push_back(KRecentFilesModelPrivate::RecentFileInfo{title, url, QMimeDatabase().mimeTypeForName(mimeTypeStr)});
    endInsertRows();
}

void KRecentFilesModel::removeAt(const QModelIndex &index)
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return;
    }

    Q_D(KRecentFilesModel);
    beginRemoveRows({}, index.row(), index.row());
    d->m_recentFiles.erase(d->m_recentFiles.begin() + d->m_recentFiles.size() - index.row() - 1);
    endRemoveRows();
}

void KRecentFilesModel::removeUrl(const QUrl &url)
{
    if (const QModelIndex index = indexOfUrl(url); index.isValid()) {
        removeAt(index);
    }
}

QModelIndex KRecentFilesModel::indexOfUrl(const QUrl &url) const
{
    Q_D(const KRecentFilesModel);
    auto it = std::find_if(d->m_recentFiles.cbegin(), d->m_recentFiles.cend(), [&url](const auto &item) {
        return item.url == url;
    });
    if (it == d->m_recentFiles.cend()) {
        return {};
    }
    return index(std::distance(it, d->m_recentFiles.cend()) - 1);
}

QList<QUrl> KRecentFilesModel::urls() const
{
    Q_D(const KRecentFilesModel);

    QList<QUrl> list;
    list.reserve(d->m_recentFiles.size());

    std::transform(d->m_recentFiles.crbegin(), d->m_recentFiles.crend(), std::back_inserter(list), [](const auto &item) {
        return item.url;
    });

    return list;
}

void KRecentFilesModel::clear()
{
    Q_D(KRecentFilesModel);
    if (d->m_recentFiles.empty()) {
        return;
    }

    beginResetModel();
    d->m_recentFiles.clear();
    endResetModel();
}

void KRecentFilesModel::loadEntries(const KConfigGroup &_config)
{
    Q_D(KRecentFilesModel);

    beginResetModel();
    d->m_recentFiles.clear();

    QString value;
    QString title;
    QUrl url;
    QMimeDatabase db;

    KConfigGroup cg = _config;
    // "<default>" means the group was constructed with an empty name
    if (cg.name() == QLatin1String("<default>")) {
        cg = KConfigGroup(cg.config(), QStringLiteral("RecentFiles"));
    }

    std::byte memory[sizeof(QUrl) * 32];
    std::pmr::monotonic_buffer_resource resource(memory, sizeof(memory));
    std::pmr::set<QUrl> seenUrls(&resource);

    // read file list
    for (int i = 1; i <= d->m_maxItems; i++) {
        char key[128]{};
        std::snprintf(key, sizeof key, "File%d", i);
        value = cg.readPathEntry(key, QString());
        if (value.isEmpty()) {
            continue;
        }
        url = QUrl::fromUserInput(value);

        auto [it, isNewUrl] = seenUrls.insert(url);
        // Don't restore if this url has already been restored (e.g. broken config)
        if (!isNewUrl) {
            continue;
        }

#ifdef Q_OS_WIN
        // convert to backslashes
        if (url.isLocalFile()) {
            value = QDir::toNativeSeparators(value);
        }
#endif

        char nameKey[128]{};
        std::snprintf(nameKey, sizeof nameKey, "Name%d", i);

        title = cg.readPathEntry(nameKey, url.fileName());

        char mimeTypeKey[128]{};
        std::snprintf(mimeTypeKey, sizeof mimeTypeKey, "MimeType%d", i);
        QString mimeTypeName = cg.readEntry(mimeTypeKey, QString());

        QMimeType mimeType;
        if (!mimeTypeName.isEmpty()) {
            mimeType = db.mimeTypeForName(mimeTypeName);
        }

        KRecentFilesModelPrivate::RecentFileInfo item{title, url, mimeType};
        d->m_recentFiles.push_back(item);
    }

    endResetModel();
}

void KRecentFilesModel::saveEntries(const KConfigGroup &_cg)
{
    Q_D(KRecentFilesModel);

    KConfigGroup cg = _cg;
    // "<default>" means the group was constructed with an empty name
    if (cg.name() == QLatin1String("<default>")) {
        cg = KConfigGroup(cg.config(), QStringLiteral("RecentFiles"));
    }

    cg.deleteGroup();

    // write file list
    int i = 1;
    for (const auto &[title, url, mimeType] : d->m_recentFiles) {
        char key[128]{};
        std::snprintf(key, sizeof key, "File%d", i);
        cg.writePathEntry(key, url.toDisplayString(QUrl::PreferLocalFile));

        char nameKey[128]{};
        std::snprintf(nameKey, sizeof nameKey, "Name%d", i);
        cg.writePathEntry(nameKey, title);

        char mimeTypeKey[128]{};
        std::snprintf(mimeTypeKey, sizeof mimeTypeKey, "MimeType%d", i);
        if (mimeType.isValid() && !mimeType.isDefault()) {
            cg.writeEntry(mimeTypeKey, mimeType.name());
        } else {
            cg.deleteEntry(mimeTypeKey);
        }

        ++i;
    }
}

int KRecentFilesModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const KRecentFilesModel);
    return parent.isValid() ? 0 : d->m_recentFiles.size();
}

// KShell::tildeCollapse
static QString tildeCollapse(const QString &path)
{
    if (!path.isEmpty()) {
        const auto homePath = QDir::homePath();
        if (path.startsWith(homePath)) {
            auto newPath = path;
            newPath.replace(0, homePath.length(), QLatin1Char('~'));
            return newPath;
        }
    }
    return path;
}

QVariant KRecentFilesModel::data(const QModelIndex &index, int role) const
{
    Q_D(const KRecentFilesModel);
    if (!index.isValid() || index.row() >= static_cast<int>(d->m_recentFiles.size())) {
        return {};
    }

    auto &item = d->m_recentFiles[d->m_recentFiles.size() - index.row() - 1];

    switch (role) {
    case Qt::DisplayRole: {
        const QString path = tildeCollapse(item.url.toDisplayString(QUrl::PreferLocalFile | QUrl::StripTrailingSlash | QUrl::RemoveFilename));
        return tr("%1 @ %2", "recent file name @ path").arg(item.title, path);
    }
    case Qt::ToolTipRole:
        return tildeCollapse(item.url.toDisplayString(QUrl::PreferLocalFile));
    case Qt::DecorationRole:
        // Set icons lazily based on the mimetype.
        if (!item.mimeType.isValid()) {
            item.mimeType = QMimeDatabase().mimeTypeForFile(item.url.path(), QMimeDatabase::MatchExtension);
        }
        if (item.mimeType.isValid() && !item.mimeType.isDefault()) {
            return QIcon::fromTheme(item.mimeType.iconName());
        }
        return {};
    case TitleRole:
        return item.title;
    case UrlRole:
        return item.url;
    }

    return {};
}

QHash<int, QByteArray> KRecentFilesModel::roleNames() const
{
    auto roles = QAbstractListModel::roleNames();
    roles[TitleRole] = "title"_ba;
    roles[UrlRole] = "url"_ba;
    return roles;
}

#include "moc_krecentfilesmodel.cpp"
