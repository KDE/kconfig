/*
 *   Copyright 2018 David Edmundson <davidedmundson@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kconfigwatcher.h"

#include "config-kconfig.h"
#include "kconfig_core_log_settings.h"

#if KCONFIG_USE_DBUS
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#endif

#include <QDebug>
#include <QThreadStorage>
#include <QHash>

class KConfigWatcherPrivate {
public:
    KSharedConfig::Ptr m_config;
};

KConfigWatcher::Ptr KConfigWatcher::create(const KSharedConfig::Ptr &config)
{
    static QThreadStorage<QHash<KSharedConfig*, QWeakPointer<KConfigWatcher>>> watcherList;

    auto c = config.data();
    KConfigWatcher::Ptr watcher;

    if (!watcherList.localData().contains(c)) {
        watcher = KConfigWatcher::Ptr(new KConfigWatcher(config));

        watcherList.localData().insert(c, watcher.toWeakRef());

        QObject::connect(watcher.data(), &QObject::destroyed, [c]() {
            watcherList.localData().remove(c);
        });
    }
    return watcherList.localData().value(c).toStrongRef();
}

KConfigWatcher::KConfigWatcher(const KSharedConfig::Ptr &config):
    QObject (nullptr),
    d(new KConfigWatcherPrivate)
{
    Q_ASSERT(config);
#if KCONFIG_USE_DBUS

    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<QHash<QString, QByteArrayList>>();

    d->m_config = config;

    QStringList watchedPaths;
    watchedPaths << QLatin1Char('/') + d->m_config->name();
    for (const QString &file: d->m_config->additionalConfigSources()) {
        watchedPaths << QLatin1Char('/') + file;
    }
    if (d->m_config->openFlags() & KConfig::IncludeGlobals) {
        watchedPaths << QStringLiteral("/kdeglobals");
    }

    for(const QString &path: qAsConst(watchedPaths)) {
        QDBusConnection::sessionBus().connect(QString(),
                                              path,
                                              QStringLiteral("org.kde.kconfig.notify"),
                                              QStringLiteral("ConfigChanged"),
                                              this,
                                              SLOT(onConfigChangeNotification(QHash<QString,QByteArrayList>)));
    }
#else
    qCWarning(KCONFIG_CORE_LOG) << "Use of KConfigWatcher without DBus support. You will not receive updates";
#endif
}

KConfigWatcher::~KConfigWatcher() = default;

void KConfigWatcher::onConfigChangeNotification(const QHash<QString, QByteArrayList> &changes)
{
    //should we ever need it we can determine the file changed with  QDbusContext::message().path(), but it doesn't seem too useful

    d->m_config->reparseConfiguration();

    for(auto it = changes.constBegin(); it != changes.constEnd(); it++) {
        KConfigGroup group = d->m_config->group(QString());//top level group
        const auto parts = it.key().split(QLatin1Char('\x1d')); //magic char, see KConfig
        for(const QString &groupName: parts) {
            group = group.group(groupName);
        }
        emit configChanged(group, it.value());
    }
}

