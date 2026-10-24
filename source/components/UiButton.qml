import QtQuick
import QtQuick.Controls
import "."

Button {
    id: control

    property string glyph
    // primary | secondary | ghost | danger
    property string variant: "secondary"

    readonly property color foreground: !enabled ? Theme.textDisabled
        : variant === "primary" ? Theme.accentForeground
        : variant === "danger" ? Theme.danger
        : Theme.text

    implicitHeight: Theme.controlHeight
    leftPadding: text.length > 0 ? 14 : 0
    rightPadding: text.length > 0 ? 14 : 0
    hoverEnabled: true
    font.pixelSize: Theme.fontBody
    font.weight: Font.Medium

    contentItem: Item {
        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight
        Row {
            id: row
            anchors.centerIn: parent
            spacing: 7
            Icon {
                visible: control.glyph.length > 0
                anchors.verticalCenter: parent.verticalCenter
                glyph: control.glyph
                size: 13
                color: control.foreground
            }
            Label {
                visible: control.text.length > 0
                anchors.verticalCenter: parent.verticalCenter
                text: control.text
                font: control.font
                color: control.foreground
            }
        }
    }

    background: Rectangle {
        implicitWidth: control.text.length > 0 ? 72 : Theme.controlHeight
        implicitHeight: Theme.controlHeight
        radius: Theme.radiusMedium
        color: {
            if (control.variant === "primary")
                return !control.enabled ? Theme.control
                     : control.down ? Theme.accentPressed
                     : control.hovered ? Theme.accentHover : Theme.accent
            if (control.variant === "danger")
                return control.down ? "#33f2777a" : control.hovered ? "#29f2777a" : Theme.dangerSoft
            if (control.variant === "ghost")
                return control.down ? Theme.controlPressed : control.hovered ? Theme.controlHover : "transparent"
            return control.down ? Theme.controlPressed : control.hovered ? Theme.controlHover : Theme.control
        }
        border.width: control.visualFocus ? 2 : (control.variant === "secondary" ? 1 : 0)
        border.color: control.visualFocus ? Theme.accent : Theme.border
        Behavior on color { ColorAnimation { duration: 110 } }
    }
}
