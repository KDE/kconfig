// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "propertystatesaver.h"
#include "kconfig_qml_log_settings.h"

#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWindow>

#include <KConfigGroup>
#include <KSharedConfig>

PropertyStateSaver::PropertyStateSaver(QObject *parent)
    : QObject(parent)
{
}

void PropertyStateSaver::onPropertyChanged()
{
    // const QVariant newProperty = m_target->property(m_propertyName.toLatin1().data());
    const QQmlProperty prop(m_target, m_propertyName);
    KConfigGroup cg(KSharedConfig::openStateConfig(), m_configGroupName);
    cg.writeEntry(m_keyName, prop.read());
}

QObject *PropertyStateSaver::target() const
{
    return m_target;
}

void PropertyStateSaver::setTarget(QObject *target)
{
    if (m_target == target) {
        return;
    }

    if (m_target) {
        disconnect(m_target, nullptr, this, nullptr);
    }

    m_target = target;

    reconnect();
    Q_EMIT targetChanged();
}

QString PropertyStateSaver::configGroupName() const
{
    return m_configGroupName;
}

void PropertyStateSaver::setConfigGroupName(const QString &group)
{
    if (m_configGroupName == group) {
        return;
    }

    m_configGroupName = group;

    reconnect();
    Q_EMIT configGroupNameChanged();
}

QString PropertyStateSaver::configKey() const
{
    return m_keyName;
}

void PropertyStateSaver::setConfigKey(const QString &key)
{
    if (m_keyName == key) {
        return;
    }

    m_keyName = key;

    reconnect();
    Q_EMIT configKeyChanged();
}

QString PropertyStateSaver::property() const
{
    return m_propertyName;
}

void PropertyStateSaver::setProperty(const QString &property)
{
    if (m_propertyName == property) {
        return;
    }

    m_propertyName = property;

    reconnect();
    Q_EMIT propertyChanged();
}

void PropertyStateSaver::reconnect()
{
    if (!m_target || m_configGroupName.isEmpty() || m_propertyName.isEmpty() || m_keyName.isEmpty()) {
        return;
    }

    disconnect(m_target, nullptr, this, nullptr);

    const QQmlProperty prop(m_target, m_propertyName);
    if (prop.isValid()) {
        prop.connectNotifySignal(this, SLOT(onPropertyChanged()));

        KConfigGroup cg(KSharedConfig::openStateConfig(), m_configGroupName);

        if (cg.hasKey(m_keyName)) {
            QVariant var = prop.read();
            switch (var.typeId()) {
            case QMetaType::Int:
                prop.write(cg.readEntry(m_keyName, 0));
                break;
            case QMetaType::Double:
                prop.write(cg.readEntry(m_keyName, 0.0));
                break;
            case QMetaType::QString:
                prop.write(cg.readEntry(m_keyName, QString()));
                break;
            case QMetaType::QStringList:
                prop.write(cg.readEntry(m_keyName, QStringList()));
                break;
            case QMetaType::QByteArray:
                prop.write(cg.readEntry(m_keyName, QByteArray()));
                break;
            }
        }
    }
}

#include "moc_propertystatesaver.cpp"
