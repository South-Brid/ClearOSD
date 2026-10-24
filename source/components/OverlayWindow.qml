import QtQuick

Window {
    required property var controller
    id: overlay
    objectName: "overlayWindow"
    // Independent top-level HUD: minimizing/hiding settings must not hide it.
    transientParent: null
    title: "ClearOSD Overlay"
    // The setting is physical pixels, matching RTSS's FNT height at zoom 1.
    property real pixelRatio: Screen.devicePixelRatio
    width: strip.implicitWidth
    height: strip.implicitHeight
    visible: controller.desktopVisible
    color: "transparent"
    flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | (controller.config.locked ? Qt.WindowTransparentForInput : 0)
    MetricStrip {
        id: strip
        anchors.fill: parent
        presentation: controller.overlayStyle
        pixelRatio: overlay.pixelRatio
    }
    MouseArea {
        anchors.fill: parent; acceptedButtons: Qt.LeftButton | Qt.RightButton
        enabled: !controller.config.locked
        cursorShape: Qt.SizeAllCursor
        onPressed: function(mouse) {
            if (mouse.button === Qt.RightButton) controller.openSettings()
            else { controller.beginDrag(); overlay.startSystemMove() }
        }
    }
}
