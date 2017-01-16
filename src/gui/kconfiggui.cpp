/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kconfiggui.h"

#include <QGuiApplication>

#include <kconfig.h>

static QString configName(const QString &id, const QString &key)
{
    return(QLatin1String("session/") + QGuiApplication::applicationName() +
           QLatin1Char('_')          + id                                 +
           QLatin1Char('_')          + key);
}

static KConfig *s_sessionConfig = nullptr;

KConfig *KConfigGui::sessionConfig()
{
#ifdef QT_NO_SESSIONMANAGER
#error QT_NO_SESSIONMANAGER was set, this will not compile. Reconfigure Qt with Session management support.
#endif
    if (!hasSessionConfig()) {
        // create the default instance specific config object
        // from applications' -session command line parameter
        s_sessionConfig = new KConfig(configName(qApp->sessionId(),
                                                 qApp->sessionKey()),
                                      KConfig::SimpleConfig);
    }

    return s_sessionConfig;
}

void KConfigGui::setSessionConfig(const QString &id, const QString &key)
{
    if (hasSessionConfig()) {
        delete s_sessionConfig;
        s_sessionConfig = nullptr;
    }

    // create a new instance specific config object from supplied id & key
    s_sessionConfig = new KConfig(configName(id, key),
                                  KConfig::SimpleConfig);
}

bool KConfigGui::hasSessionConfig()
{
    return s_sessionConfig != nullptr;
}

#ifndef KDE_NO_DEPRECATED
QString KConfigGui::sessionConfigName()
{
    return sessionConfig()->name();
}
#endif
