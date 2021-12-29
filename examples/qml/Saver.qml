import QtQuick.Window 2.10
import org.kde.kconfig 1.0

Window {
    id: rootWindow

    WindowStateSaver {
        window: rootWindow
        windowName: "rootWindow"
        configName: "qml-example"
    }
}