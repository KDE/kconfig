/*
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <QHash>
#include <QThreadStorage>

class KConfigWatcherPrivate
{
public:
    KSharedConfig::Ptr m_config;
};

KConfigWatcher::Ptr KConfigWatcher::create(const KSharedConfig::Ptr &config)
{
    static QThreadStorage<QHash<KSharedConfig *, QWeakPointer<KConfigWatcher>>> watcherList;

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

KConfigWatcher::KConfigWatcher(const KSharedConfig::Ptr &config)
    : QObject(nullptr)
    , d(new KConfigWatcherPrivate)
{
    Q_ASSERT(config);
    d->m_config = config;

#if KCONFIG_USE_DBUS

    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<QHash<QString, QByteArrayList>>();


    QStringList watchedPaths = d->m_config->additionalConfigSources();
    for (QString &file : watchedPaths) {
        file.prepend(QLatin1Char('/'));
    }
    watchedPaths.prepend(QLatin1Char('/') + d->m_config->name().replace(QLatin1Char('-'), QLatin1Char('_')));

    if (d->m_config->openFlags() & KConfig::IncludeGlobals) {
        watchedPaths << QStringLiteral("/kdeglobals");
    }

    for (const QString &path : std::as_const(watchedPaths)) {
        QDBusConnection::sessionBus().connect(QString(),
                                              path,
                                              QStringLiteral("org.kde.kconfig.notify"),
                                              QStringLiteral("ConfigChanged"),
                                              this,
                                              SLOT(onConfigChangeNotification(QHash<QString, QByteArrayList>)));
    }
#else
    qCWarning(KCONFIG_CORE_LOG) << "Use of KConfigWatcher without DBus support. You will not receive updates";
#endif
}

KConfigWatcher::~KConfigWatcher() = default;

KSharedConfig::Ptr KConfigWatcher::config() const
{
    return d->m_config;
}

void KConfigWatcher::onConfigChangeNotification(const QHash<QString, QByteArrayList> &changes)
{
    // should we ever need it we can determine the file changed with  QDbusContext::message().path(), but it doesn't seem too useful

    d->m_config->reparseConfiguration();

    for (auto it = changes.constBegin(); it != changes.constEnd(); it++) {
        KConfigGroup group = d->m_config->group(QString()); // top level group
        const auto parts = it.key().split(QLatin1Char('\x1d')); // magic char, see KConfig
        for (const QString &groupName : parts) {
            group = group.group(groupName);
        }
        Q_EMIT configChanged(group, it.value());
    }
}
