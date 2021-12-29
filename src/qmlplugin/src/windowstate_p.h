// SPDX-FileCopyrightText: 2021 Carson Black
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>
#include <QQmlParserStatus>

class QWindow;

/**
 * @brief WindowStateSaver saves the geometry of the provided window
 *
 * @example qml/Saver.qml
 */
class WindowStateSaver : public QObject, public QQmlParserStatus
{

    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    struct Private;
    QScopedPointer<Private> d;

    /**
     * The window that WindowStateSaver will load and save the state of.
     */
    Q_PROPERTY(QWindow* window READ window WRITE setWindow NOTIFY windowChanged)

    /**
     * A unique name for the window that can be used to distinguish it from other
     * windows in your application.
     *
     * This is required for proper functionality.
     */
    Q_PROPERTY(QString windowName READ windowName WRITE setWindowName NOTIFY windowNameChanged)

    /**
     * The name of the config file the WindowStateSaver will read and write to.
     * This will default to the one used by the rest of your application, e.g.
     * one derived from properties of the QApplication.
     */
    Q_PROPERTY(QString configName READ configName WRITE setConfigName NOTIFY configNameChanged)

public:
    explicit WindowStateSaver(QObject* parent = nullptr);
    ~WindowStateSaver();

    QWindow* window() const;
    void setWindow(QWindow* window);
    Q_SIGNAL void windowChanged();

    QString windowName() const;
    void setWindowName(const QString& name);
    Q_SIGNAL void windowNameChanged();

    QString configName() const;
    void setConfigName(const QString& configName);
    Q_SIGNAL void configNameChanged();

    void classBegin() override { }
    void componentComplete() override;

    /**
     * Manually save the window's state. This is normally not needed
     * in normal operation, as the WindowStateSaver does this automatically.
     */
    Q_INVOKABLE void save();

    /**
     * Manually restore the window's state. This is normally not needed
     * in normal operation, as the WindowStateSaver does this automatically.
     */
    Q_INVOKABLE void restore();

};