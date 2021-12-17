// SPDX-FileCopyrightText: 2021 Carson Black
//
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick.Window 2.10
import QtQuick 2.0
import QtTest 1.2
import org.kde.kconfig 1.0

TestCase {
    id: testCase
    name: "Basic Functionality"

    function generateQML(windowName) {
        return `import QtQuick.Window 2.10
import QtQuick 2.0
import org.kde.kconfig 1.0

Window {
    id: rootWindow

    WindowStateSaver {
        window: rootWindow
        windowName: "${windowName}"
        configName: "testing-window"
    }
}
`
    }

    function test_saverestore() {
        let item = createTemporaryQmlObject(generateQML("test-00"), testCase)
        verify(item)

        const num = Math.ceil(Math.random() * 1000)

        item.visible = true
        item.x = num
        item.y = num
        item.width = num
        item.height = num

        wait(1000)

        item.destroy()

        wait(1000)

        item = createTemporaryQmlObject(generateQML("test-00"), testCase)
        verify(item)
        wait(1000)

        if (Qt.platform.pluginName !== "wayland") {
            verify(item.x == num)
            verify(item.y == num)
        }

        verify(item.width == num)
        verify(item.height == num)
    }
}
