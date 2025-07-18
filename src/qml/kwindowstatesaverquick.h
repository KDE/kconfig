// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef KWINDOWSTATESAVER_QUICK_H
#define KWINDOWSTATESAVER_QUICK_H

#include <QQmlEngine>

class PropertyObject : public QObject
{
    Q_OBJECT
public:
    PropertyObject(QObject *sender, const QString &configGroupName, const QString &property, QObject *parent = nullptr);

private Q_SLOTS:
    void propertyChanged();

private:
    QPointer<QObject> m_sender;
    QString m_configGroupName;
    QString m_propertyName;
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
    Q_PROPERTY(QStringList extraProperties READ extraProperties WRITE setExtraProperties NOTIFY extraPropertiesChanged)

public:
    void classBegin() override;
    void componentComplete() override;

    void setConfigGroupName(const QString &name);
    QString configGroupName() const;

    // TODO: perhaps is better a QQmlListProperty?
    QStringList extraProperties() const;
    void setExtraProperties(const QStringList &properties);

Q_SIGNALS:
    void configGroupNameChanged();
    void extraPropertiesChanged();

private:
    QString m_configGroupName;
    QStringList m_extraProperties;
    std::unordered_map<QString, std::unique_ptr<PropertyObject>> m_propertyObjects;
};

#endif
