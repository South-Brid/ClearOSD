import QtQuick
import QtQuick.Controls
import "."

SpinBox {
    id: control

    editable: true
    hoverEnabled: true
    implicitWidth: 136
    implicitHeight: Theme.controlHeight
    font.pixelSize: Theme.fontBody

    contentItem: TextInput {
        z: 2
        text: control.displayText
        font: control.font
        color: Theme.text
        selectionColor: Theme.accent
        selectedTextColor: Theme.accentForeground
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly
        selectByMouse: true
    }

    component StepButton: Rectangle {
        required property var indicatorState
        property string glyph
        implicitWidth: 32
        height: control.height
        radius: Theme.radiusMedium
        color: indicatorState.pressed ? Theme.controlPressed
             : indicatorState.hovered ? Theme.controlHover : "transparent"
        Icon {
            anchors.centerIn: parent
            glyph: parent.glyph
            size: 11
            color: parent.enabled ? Theme.textSecondary : Theme.textDisabled
        }
    }

    up.indicator: StepButton {
        x: control.mirrored ? 0 : control.width - width
        indicatorState: control.up
        glyph: Theme.iconAdd
        enabled: control.value < control.to
    }

    down.indicator: StepButton {
        x: control.mirrored ? control.width - width : 0
        indicatorState: control.down
        glyph: Theme.iconRemove
        enabled: control.value > control.from
    }

    background: Rectangle {
        radius: Theme.radiusMedium
        color: control.hovered ? Theme.controlHover : Theme.control
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
    }
}
