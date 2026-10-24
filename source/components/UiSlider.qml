import QtQuick
import QtQuick.Controls
import "."

Slider {
    id: control

    hoverEnabled: true
    implicitHeight: 26
    padding: 0

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: 160
        width: control.availableWidth
        height: 4
        radius: 2
        color: Theme.track

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: 2
            color: control.enabled ? Theme.accent : Theme.textDisabled
        }
    }

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: 20
        implicitHeight: 20
        radius: 10
        color: Theme.surfaceRaised
        border.width: control.visualFocus ? 2 : 1
        border.color: control.visualFocus ? Theme.accent : Theme.borderStrong

        Rectangle {
            anchors.centerIn: parent
            width: control.pressed ? 9 : control.hovered ? 13 : 11
            height: width
            radius: width / 2
            color: control.enabled ? Theme.accent : Theme.textDisabled
            Behavior on width { NumberAnimation { duration: 90 } }
        }
    }
}
