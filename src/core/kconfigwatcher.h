/*
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KCONFIGWATCHER_H
#define KCONFIGWATCHER_H

#include <QObject>
#include <QSharedPointer>

#include <KConfigGroup>
#include <KSharedConfig>

#include <kconfigcore_export.h>

class KConfigWatcherPrivate;

/*!
 * \class KConfigWatcher
 * \inmodule KConfigCore
 *
 * \brief Notifies when another client has updated this config file with the Notify flag set.
 * \since 5.51
 */
class KCONFIGCORE_EXPORT KConfigWatcher : public QObject
{
    Q_OBJECT
public:
    /*!
     * \typedef KConfigWatcher::Ptr
     */
    typedef QSharedPointer<KConfigWatcher> Ptr;

    /*!
     * Instantiate a ConfigWatcher for a given config
     *
     * \note any additional config sources should be set before this point.
     */
    static Ptr create(const KSharedConfig::Ptr &config);

    ~KConfigWatcher() override;

    /*!
     * Returns the config being watched
     * \since 5.66
     */
    KSharedConfig::Ptr config() const;

Q_SIGNALS:
    /*!
     * Emitted when a config group has changed
     * The config will be reloaded before this signal is emitted
     *
     * \a group the config group that has changed
     *
     * \a names a list of entries that have changed within that group (UTF-8 encoded)
     */
    void configChanged(const KConfigGroup &group, const QByteArrayList &names);

private Q_SLOTS:
    KCONFIGCORE_NO_EXPORT void onConfigChangeNotification(const QHash<QString, QByteArrayList> &changes);

private:
    KCONFIGCORE_NO_EXPORT explicit KConfigWatcher(const KSharedConfig::Ptr &config);
    Q_DISABLE_COPY(KConfigWatcher)
    const QScopedPointer<KConfigWatcherPrivate> d;
};

#endif
