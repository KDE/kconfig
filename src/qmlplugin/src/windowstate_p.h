#pragma once

#include <QObject>
#include <QQmlParserStatus>

class QWindow;

class WindowStateSaver : public QObject, public QQmlParserStatus
{

    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    struct Private;
    QScopedPointer<Private> d;

    Q_PROPERTY(QWindow* window READ window WRITE setWindow NOTIFY windowChanged)
    Q_PROPERTY(QString windowName READ windowName WRITE setWindowName NOTIFY windowNameChanged)
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

    Q_INVOKABLE void save();
    Q_INVOKABLE void restore();

};