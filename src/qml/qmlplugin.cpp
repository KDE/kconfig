// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

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
        qmlRegisterSingletonType<KAuthorized>(uri, 1, 0, "KAuthorized", [](QQmlEngine *, QJSEngine *) {
            return new KAuthorized();
        });
    }
};

#include "qmlplugin.moc"
