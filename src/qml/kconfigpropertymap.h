/*
    SPDX-FileCopyrightText: 2013 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGPROPERTYMAP_H
#define KCONFIGPROPERTYMAP_H

#include <QQmlPropertyMap>
#include <memory>
#include <qqmlregistration.h>

class KCoreConfigSkeleton;

#include <kconfigqml_export.h>

class KConfigPropertyMapPrivate;

/*!
 * \class KConfigPropertyMap
 * \inmodule KConfigQml
 *
 * \brief An object that (optionally) automatically saves changes in a
 * property map to a configuration object (e.g. a KConfig file).
 * \since 5.89
 */
class KCONFIGQML_EXPORT KConfigPropertyMap : public QQmlPropertyMap
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    /*!
     *
     */
    KConfigPropertyMap(KCoreConfigSkeleton *config, QObject *parent = nullptr);
    ~KConfigPropertyMap() override;

    /*!
     * Whether notifications on config changes are enabled. Disabled by default.
     *
     * Returns \c true if writes send (dbus) notifications
     *
     * \sa KConfigBase::Notify
     */
    bool isNotify() const;

    /*!
     * Enable or disable notifications on config changes.
     *
     * \a notify whether to send notifications
     *
     * \sa KConfigBase::Notify
     */
    void setNotify(bool notify);

    /*!
     * Whether the value at the given key is immutable
     *
     * Returns \c true if the value is immutable, \c false if it isn't or it doesn't exist
     */
    Q_INVOKABLE bool isImmutable(const QString &key) const;

    /*!
     * Saves the state of the property map on disk.
     */
    Q_INVOKABLE void writeConfig();

protected:
    QVariant updateValue(const QString &key, const QVariant &input) override;

private:
    std::unique_ptr<KConfigPropertyMapPrivate> const d;
};

#endif
