// SPDX-FileCopyrightText: 2021 Carson Black
//
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import org.kde.kconfig 1.0

Window {
    id: rootWindow

    color: "black"

    WindowStateSaver {
        window: rootWindow
        windowName: "Main Window"
        configName: "kconfigqmlpluginexample"
    }

    Text {
        anchors.centerIn: parent
        color: "white"
        text: "If you close and open me, I'll be where I was!"
    }
}