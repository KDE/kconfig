// SPDX-FileCopyrightText: 2021 Carson Black
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QQmlEngine>

#include "plugin_p.h"
#include "windowstate_p.h"

void KConfigQMLPlugin::registerTypes(const char* uri)
{
    qmlRegisterType<WindowStateSaver>(uri, 1, 0, "WindowStateSaver");
}
