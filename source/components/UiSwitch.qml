import QtQuick
import QtQuick.Controls
import "."

Switch {
    id: control

    hoverEnabled: true
    spacing: 10
    padding: 0
    font.pixelSize: Theme.fontBody

    indicator: Rectangle {
        implicitWidth: 40
        implicitHeight: 22
        x: control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        radius: height / 2
        opacity: control.enabled ? 1 : 0.45
        color: control.checked ? (control.hovered ? Theme.accentHover : Theme.accent)
                               : (control.hovered ? Theme.controlHover : Theme.control)
        border.width: control.checked ? 0 : 1
        border.color: control.visualFocus ? Theme.accent : Theme.borderStrong
        Behavior on color { ColorAnimation { duration: 120 } }

        Rectangle {
            width: control.pressed ? 18 : 16
            height: 16
            radius: 8
            y: 3
            x: control.checked ? parent.width - width - 3 : 3
            color: control.checked ? Theme.accentForeground : Theme.textSecondary
            Behavior on x { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
            Behavior on width { NumberAnimation { duration: 90 } }
        }
    }

    contentItem: Label {
        leftPadding: control.indicator.width + (control.text.length > 0 ? control.spacing : 0)
        text: control.text
        font: control.font
        color: control.enabled ? Theme.textSecondary : Theme.textDisabled
        verticalAlignment: Text.AlignVCenter
    }
}
