/*
    SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: MIT
*/

import QtQuick
import org.kde.kconfig.test19

Item {
    property url dir: Test19.picturesDir
    property double brightness: Test19.brightness
    property double brightnessDefault: Test19.defaultBrightnessValue
}
