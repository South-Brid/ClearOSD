import QtQuick

Rectangle {
    id: strip
    required property var presentation
    property real pixelRatio: 1
    readonly property real paddingX: 12 / pixelRatio
    readonly property real paddingY: 6 / pixelRatio
    implicitWidth: Math.ceil(metrics.implicitWidth + 2 * paddingX)
    implicitHeight: Math.ceil(metrics.implicitHeight + 2 * paddingY)
    color: Qt.rgba(Qt.color(presentation.backgroundColor).r,
                   Qt.color(presentation.backgroundColor).g,
                   Qt.color(presentation.backgroundColor).b,
                   presentation.backgroundOpacity)
    Text {
        id: metrics
        anchors.centerIn: parent
        text: strip.presentation.text
        color: strip.presentation.textColor
        font.family: strip.presentation.fontFamily
        font.pointSize: strip.presentation.fontSize * 72 / (Screen.logicalPixelDensity * 25.4 * strip.pixelRatio)
        font.weight: strip.presentation.fontWeight
        renderType: Text.NativeRendering
    }
}
