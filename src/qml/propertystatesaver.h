// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef PROPERTYSTATESAVER_H
#define PROPERTYSTATESAVER_H

#include <QQmlEngine>

class PropertyStateSaver : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(PropertyStateSaver)

    Q_PROPERTY(QObject *target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(QString configGroupName READ configGroupName WRITE setConfigGroupName NOTIFY configGroupNameChanged REQUIRED)
    Q_PROPERTY(QString configKey READ configKey WRITE setConfigKey NOTIFY configKeyChanged)
    Q_PROPERTY(QString property READ property WRITE setProperty NOTIFY propertyChanged)

public:
    PropertyStateSaver(QObject *parent = nullptr);

    QObject *target() const;
    void setTarget(QObject *target);

    QString configGroupName() const;
    void setConfigGroupName(const QString &group);

    QString configKey() const;
    void setConfigKey(const QString &key);

    QString property() const;
    void setProperty(const QString &property);

private Q_SLOTS:
    void onPropertyChanged();

Q_SIGNALS:
    void targetChanged();
    void configGroupNameChanged();
    void configKeyChanged();
    void propertyChanged();

private:
    void reconnect();

    QPointer<QObject> m_target;
    QString m_configGroupName;
    QString m_propertyName;
    QString m_keyName;
};

#endif
