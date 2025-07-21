// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kwindowstatesaverquick.h"
#include "kconfig_qml_log_settings.h"

#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWindow>

#include <KSharedConfig>
#include <KWindowStateSaver>

PropertyObject::PropertyObject(QObject *parent)
    : QObject(parent)
{
}

void PropertyObject::classBegin()
{
}

void PropertyObject::componentComplete()
{
    const auto windowStateSaver = qobject_cast<KWindowStateSaverQuick *>(parent());
    if (!windowStateSaver) {
        qCWarning(KCONFIG_QML_LOG) << "PropertyStateSaver can only be a child of WindowStateSaver";
        return;
    }
}

void PropertyObject::onPropertyChanged()
{

    //const QVariant newProperty = m_target->property(m_propertyName.toLatin1().data());
    const QQmlProperty prop(m_target, m_propertyName);
    KConfigGroup cg(KSharedConfig::openStateConfig(), m_configGroupName);
    cg.writeEntry(m_keyName, prop.read());
}


QObject *PropertyObject::target() const
{
    return m_target;
}

void PropertyObject::setTarget(QObject *target)
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

QString PropertyObject::configGroupName() const
{
    return m_configGroupName;
}

void PropertyObject::setConfigGroupName(const QString &group)
{
    if (m_configGroupName == group) {
        return;
    }

    m_configGroupName = group;

    reconnect();
    Q_EMIT configGroupNameChanged();
}

QString PropertyObject::configKey() const
{
    return m_keyName;
}

void PropertyObject::setConfigKey(const QString &key)
{
    if (m_keyName == key) {
        return;
    }

    m_keyName = key;

    reconnect();
    Q_EMIT configKeyChanged();
}

QString PropertyObject::property() const
{
    return m_propertyName;
}

void PropertyObject::setProperty(const QString &property)
{
    if (m_propertyName == property) {
        return;
    }

    m_propertyName = property;

    reconnect();
    Q_EMIT propertyChanged();
}

void PropertyObject::reconnect()
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



void KWindowStateSaverQuick::classBegin()
{
}

void KWindowStateSaverQuick::componentComplete()
{
    const auto parentItem = qobject_cast<QQuickItem *>(parent());
    if (!parentItem) {
        qCWarning(KCONFIG_QML_LOG) << "WindowStateSaver requires a parent item";
        return;
    }

    const auto window = qobject_cast<QWindow *>(parentItem->window());
    if (!window) {
        qCWarning(KCONFIG_QML_LOG) << "WindowStateSaver requires the parent to be a type that inherits QWindow";
        return;
    }

    new KWindowStateSaver(window, m_configGroupName);

    // To work around oddities in QtQuick window visibility handling.
    // If we do not set the window visible now, then our window state is
    // overwritten during QQuickWindowQmlImpl::setWindowVisibility() because
    // QQuickWindow is AutomaticVisibility by default.
    if (window->windowState() == Qt::WindowMaximized) {
        window->setVisibility(QWindow::Visibility::Maximized);
    }
}

void KWindowStateSaverQuick::setConfigGroupName(const QString &name)
{
    if (m_configGroupName != name) {
        m_configGroupName = name;
        Q_EMIT configGroupNameChanged();
    }
}

QString KWindowStateSaverQuick::configGroupName() const
{
    return m_configGroupName;
}

QQmlListProperty<PropertyObject> KWindowStateSaverQuick::extraProperties()
{
    return QQmlListProperty<PropertyObject>(this, //
                                        nullptr,
                                        extraProperties_append,
                                        extraProperties_count,
                                        extraProperties_at,
                                        extraProperties_clear);
}


void KWindowStateSaverQuick::extraProperties_append(QQmlListProperty<PropertyObject> *prop, PropertyObject *object)
{
    KWindowStateSaverQuick *saver = static_cast<KWindowStateSaverQuick *>(prop->object);
    if (!saver) {
        return;
    }

    saver->m_propertyObjects.append(object);
}

qsizetype KWindowStateSaverQuick::extraProperties_count(QQmlListProperty<PropertyObject> *prop)
{
    KWindowStateSaverQuick *saver = static_cast<KWindowStateSaverQuick *>(prop->object);
    if (!saver) {
        return 0;
    }

    return saver->m_propertyObjects.count();
}

PropertyObject *KWindowStateSaverQuick::extraProperties_at(QQmlListProperty<PropertyObject> *prop, qsizetype index)
{
    KWindowStateSaverQuick *saver = static_cast<KWindowStateSaverQuick *>(prop->object);
    if (!saver) {
        return nullptr;
    }

    if (index < 0 || index >= saver->m_propertyObjects.count()) {
        return nullptr;
    }

    return saver->m_propertyObjects[index];
}

void KWindowStateSaverQuick::extraProperties_clear(QQmlListProperty<PropertyObject> *prop)
{
    KWindowStateSaverQuick *saver = static_cast<KWindowStateSaverQuick *>(prop->object);
    if (!saver) {
        return;
    }

    saver->m_propertyObjects.clear();
}

#include "moc_kwindowstatesaverquick.cpp"
