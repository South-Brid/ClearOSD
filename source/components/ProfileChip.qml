import QtQuick
import QtQuick.Controls
import "."

AbstractButton {
    id: chip

    property string hotkey
    property bool selected: false

    implicitHeight: 34
    hoverEnabled: true
    leftPadding: 14
    rightPadding: hotkey.length > 0 ? 6 : 14

    contentItem: Row {
        spacing: 8
        Label {
            anchors.verticalCenter: parent.verticalCenter
            text: chip.text
            color: chip.selected ? Theme.accent : Theme.text
            font.pixelSize: Theme.fontBody
            font.weight: chip.selected ? Font.DemiBold : Font.Normal
        }
        Rectangle {
            visible: chip.hotkey.length > 0
            anchors.verticalCenter: parent.verticalCenter
            implicitWidth: hotkeyLabel.implicitWidth + 12
            implicitHeight: 22
            radius: 11
            color: chip.selected ? "#335bd6a0" : Theme.surfaceRaised
            Label {
                id: hotkeyLabel
                anchors.centerIn: parent
                text: Theme.formatHotkey(chip.hotkey)
                color: chip.selected ? Theme.accent : Theme.textMuted
                font.pixelSize: Theme.fontTiny
                font.family: "Consolas"
            }
        }
    }

    background: Rectangle {
        radius: height / 2
        color: chip.selected ? Theme.accentSoft : chip.down ? Theme.controlPressed
             : chip.hovered ? Theme.controlHover : Theme.control
        border.width: chip.selected || chip.visualFocus ? 1.5 : 1
        border.color: chip.selected || chip.visualFocus ? Theme.accent : Theme.border
        Behavior on color { ColorAnimation { duration: 110 } }
    }
}
