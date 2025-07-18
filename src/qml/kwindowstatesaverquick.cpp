// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kwindowstatesaverquick.h"
#include "kconfig_qml_log_settings.h"

#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWindow>

#include <KSharedConfig>
#include <KWindowStateSaver>

PropertyObject::PropertyObject(QObject *sender, const QString &configGroupName, const QString &property, QObject *parent)
    : QObject(parent)
    , m_sender(sender)
    , m_configGroupName(configGroupName)
    , m_propertyName(property)
{
    const QQmlProperty prop(sender, property);
    if (prop.isValid()) {
        prop.connectNotifySignal(this, SLOT(propertyChanged()));

        KConfigGroup cg(KSharedConfig::openStateConfig(), m_configGroupName);

        if (cg.hasKey(m_propertyName)) {
            QVariant var = prop.read();

            switch (var.typeId()) {
            case QMetaType::Int:
                prop.write(cg.readEntry(m_propertyName, 0));
                break;
            case QMetaType::Double:
                prop.write(cg.readEntry(m_propertyName, 0.0));
                break;
            case QMetaType::QString:
                prop.write(cg.readEntry(m_propertyName, QString()));
                break;
            case QMetaType::QStringList:
                prop.write(cg.readEntry(m_propertyName, QStringList()));
                break;
            case QMetaType::QByteArray:
                prop.write(cg.readEntry(m_propertyName, QByteArray()));
                break;
            }
        }
    }
}

void PropertyObject::propertyChanged()
{
    const QVariant newProperty = m_sender->property(m_propertyName.toLatin1().data());
    KConfigGroup cg(KSharedConfig::openStateConfig(), m_configGroupName);
    cg.writeEntry(m_propertyName, newProperty);
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

QStringList KWindowStateSaverQuick::extraProperties() const
{
    return m_extraProperties;
}

void KWindowStateSaverQuick::setExtraProperties(const QStringList &properties)
{
    if (m_extraProperties == properties) {
        return;
    }

    m_propertyObjects.clear();

    m_extraProperties = properties;

    const auto parentItem = qobject_cast<QQuickItem *>(parent());
    const auto window = parentItem ? qobject_cast<QWindow *>(parentItem->window()) : nullptr;

    if (window && !m_extraProperties.isEmpty()) {
        for (const QString &propName : std::as_const(m_extraProperties)) {
            m_propertyObjects[propName] = std::make_unique<PropertyObject>(window, m_configGroupName, propName);
        }
    }

    Q_EMIT extraPropertiesChanged();
}

#include "moc_kwindowstatesaverquick.cpp"
