import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

AbstractButton {
    id: control

    property color swatch: "#ffffff"

    implicitHeight: Theme.controlHeight
    implicitWidth: 160
    hoverEnabled: true
    leftPadding: 8
    rightPadding: 12

    contentItem: RowLayout {
        spacing: 10
        Rectangle {
            implicitWidth: 22
            implicitHeight: 22
            radius: 6
            color: control.swatch
            border.color: "#40ffffff"
        }
        Label {
            Layout.fillWidth: true
            text: control.swatch.toString().toUpperCase()
            color: Theme.text
            font.pixelSize: Theme.fontBody
            font.family: "Consolas"
        }
        Icon {
            glyph: Theme.iconEdit
            size: 12
            color: control.hovered ? Theme.accent : Theme.textMuted
        }
    }

    background: Rectangle {
        radius: Theme.radiusMedium
        color: control.down ? Theme.controlPressed : control.hovered ? Theme.controlHover : Theme.control
        border.width: control.visualFocus ? 2 : 1
        border.color: control.visualFocus ? Theme.accent : Theme.border
    }
}
