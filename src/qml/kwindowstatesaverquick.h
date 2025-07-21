// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef KWINDOWSTATESAVER_QUICK_H
#define KWINDOWSTATESAVER_QUICK_H

#include <QQmlEngine>

class PropertyObject : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(PropertyStateSaver)
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QObject *target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(QString configGroupName READ configGroupName WRITE setConfigGroupName NOTIFY configGroupNameChanged)
    Q_PROPERTY(QString configKey READ configKey WRITE setConfigKey NOTIFY configKeyChanged)
    Q_PROPERTY(QString property READ property WRITE setProperty NOTIFY propertyChanged)

public:
    PropertyObject(QObject *parent = nullptr);

    void classBegin() override;
    void componentComplete() override;

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

/*!
 * \qmltype WindowStateSaver
 * \inqmlmodule org.kde.config
 *
 * \brief Creates a KWindowStateSaver in QML, and assigns it to the window it's parented to.
 *
 * Functions exactly as KWindowStateSaver in C++, as it's a small wrapper around it.
 *
 * \code
 * import org.kde.config as KConfig
 *
 * Kirigami.ApplicationWindow {
 *     id: root
 *
 *     title: i18n("My Window")
 *
 *     KConfig.WindowStateSaver {
 *         configGroupName: "Main"
 *     }
 * }
 * \endcode
 * \since 6.5
 *
 * \sa KWindowStateSaver
 */
class KWindowStateSaverQuick : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(WindowStateSaver)
    Q_INTERFACES(QQmlParserStatus)

    /*!
     * \qmlproperty string WindowStateSaver::configGroupName
     */
    Q_PROPERTY(QString configGroupName READ configGroupName WRITE setConfigGroupName NOTIFY configGroupNameChanged REQUIRED)

    /*!
     * \qmlproperty Array extraProperties
     *
     * A list of extra properties of the ApplicationWindow we want to store in the state
     */
    Q_PROPERTY(QQmlListProperty<PropertyObject> extraProperties READ extraProperties)


public:
    void classBegin() override;
    void componentComplete() override;

    void setConfigGroupName(const QString &name);
    QString configGroupName() const;

    QQmlListProperty<PropertyObject> extraProperties();

Q_SIGNALS:
    void configGroupNameChanged();

private:
    static void extraProperties_append(QQmlListProperty<PropertyObject> *prop, PropertyObject *object);
    static qsizetype extraProperties_count(QQmlListProperty<PropertyObject> *prop);
    static PropertyObject *extraProperties_at(QQmlListProperty<PropertyObject> *prop, qsizetype index);
    static void extraProperties_clear(QQmlListProperty<PropertyObject> *prop);

    QString m_configGroupName;
    QQmlListProperty<PropertyObject> m_extraProperties;
    QList<PropertyObject *> m_propertyObjects;
};

#endif
