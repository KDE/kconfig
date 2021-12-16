// SPDX-FileCopyrightText: 2021 Carson Black
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QDir>
#include <QQmlExtensionPlugin>

class KConfigQMLPlugin : public QQmlExtensionPlugin
{

    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char* uri) override;

private:

/*
 * Not needed since we have no QML files currently, but may be useful if
 * someone adds QML files down the line. Leaving them here so this code is
 * conveniently already correctly written out.
 */
#if 0
    QString resolveFile(const QString &path) const
    {
        return QDir::cleanPath(baseUrl().toLocalFile() + QLatin1Char('/') + path);
    }
#endif

};