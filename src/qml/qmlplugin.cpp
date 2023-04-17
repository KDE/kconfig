#include <kauthorized.h>

#include <QJSEngine>
#include <QQmlExtensionPlugin>
#include <qqml.h>

class KConfigQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qWarning() << Q_FUNC_INFO;
        Q_ASSERT(QString::fromLatin1(uri) == QLatin1String("org.kde.kconfig"));

        qmlRegisterSingletonType<KAuthorized>(uri, 1, 0, "KAuthorized", [](QQmlEngine *, QJSEngine *) {
            return new KAuthorized();
        });
    }
};

#include "qmlplugin.moc"
