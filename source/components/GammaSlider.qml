import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

ColumnLayout {
    id: root

    property string label
    property real currentValue: 0
    property real defaultValue: 0
    property real from: 0
    property real to: 1
    property real stepSize: 0.01
    property int decimals: 2

    signal changeRequested(real value)
    signal resetRequested()

    spacing: 4

    RowLayout {
        Layout.fillWidth: true
        spacing: 8

        Label {
            text: root.label
            color: Theme.text
            font.pixelSize: Theme.fontBody
            font.weight: Font.Medium
        }
        Label {
            Layout.fillWidth: true
            text: "默认 " + Number(root.defaultValue).toFixed(root.decimals)
            color: Theme.textMuted
            font.pixelSize: Theme.fontTiny
        }

        TextField {
            id: valueField
            implicitWidth: 72
            implicitHeight: 30
            horizontalAlignment: TextInput.AlignHCenter
            verticalAlignment: TextInput.AlignVCenter
            selectByMouse: true
            hoverEnabled: true
            color: Theme.text
            selectionColor: Theme.accent
            selectedTextColor: Theme.accentForeground
            font.pixelSize: Theme.fontBody
            font.family: "Consolas"
            text: Number(root.currentValue).toFixed(root.decimals)
            validator: DoubleValidator {
                locale: "C"
                bottom: root.from
                top: root.to
                decimals: root.decimals
                notation: DoubleValidator.StandardNotation
            }
            onAccepted: {
                if (acceptableInput)
                    root.changeRequested(Number(text))
                focus = false
            }
            onEditingFinished: {
                if (acceptableInput)
                    root.changeRequested(Number(text))
                // Editing must not permanently replace the parameter binding.
                text = Qt.binding(function() {
                    return Number(root.currentValue).toFixed(root.decimals)
                })
            }
            background: Rectangle {
                radius: 8
                color: valueField.activeFocus ? Theme.surfaceRaised
                     : valueField.hovered ? Theme.controlHover : Theme.control
                border.width: valueField.activeFocus ? 2 : 1
                border.color: valueField.activeFocus ? Theme.accent : Theme.border
            }
        }

        UiButton {
            implicitWidth: 30
            implicitHeight: 30
            variant: "ghost"
            glyph: Theme.iconReset
            enabled: Math.abs(root.currentValue - root.defaultValue) > 1e-6
            ToolTip.visible: hovered
            ToolTip.delay: 400
            ToolTip.text: "恢复默认值"
            onClicked: root.resetRequested()
        }
    }

    UiSlider {
        Layout.fillWidth: true
        from: root.from
        to: root.to
        stepSize: root.stepSize
        value: root.currentValue
        onMoved: root.changeRequested(value)
    }
}
