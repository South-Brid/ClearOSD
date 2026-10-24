import QtQuick
import QtQuick.Controls
import "."

TextField {
    id: control

    implicitHeight: Theme.controlHeight
    leftPadding: 12
    rightPadding: 12
    hoverEnabled: true
    selectByMouse: true
    color: Theme.text
    placeholderTextColor: Theme.textMuted
    selectionColor: Theme.accent
    selectedTextColor: Theme.accentForeground
    font.pixelSize: Theme.fontBody
    verticalAlignment: TextInput.AlignVCenter

    background: Rectangle {
        implicitWidth: 160
        radius: Theme.radiusMedium
        color: control.activeFocus ? Theme.surfaceRaised
             : control.hovered ? Theme.controlHover : Theme.control
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
    }
}
