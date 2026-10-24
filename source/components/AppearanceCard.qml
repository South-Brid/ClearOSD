import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

SectionCard {
    id: card
    required property var controller
    signal pickColor(string field, color value)

    title: "外观"
    subtitle: "浮层的文字、背景与字号"
    glyph: Theme.iconColor

    GridLayout {
        Layout.fillWidth: true
        columns: 2
        columnSpacing: 20
        rowSpacing: 18

        Field {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            label: "文字颜色"
            ColorSwatchButton {
                Layout.fillWidth: true
                swatch: card.controller.config.textColor
                onClicked: card.pickColor("textColor", card.controller.config.textColor)
            }
        }

        Field {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            label: "背景颜色"
            ColorSwatchButton {
                Layout.fillWidth: true
                swatch: card.controller.config.backgroundColor
                onClicked: card.pickColor("backgroundColor", card.controller.config.backgroundColor)
            }
        }

        Field {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            label: "字号（物理像素）"
            UiStepper {
                Layout.fillWidth: true
                from: 10
                to: 40
                value: card.controller.config.fontSize
                onValueModified: card.controller.change("fontSize", value)
            }
        }

        Field {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            label: "背景不透明度"
            value: Math.round(card.controller.config.backgroundOpacity * 100) + "%"
            UiSlider {
                Layout.fillWidth: true
                Layout.preferredHeight: Theme.controlHeight
                from: 0
                to: 1
                value: card.controller.config.backgroundOpacity
                onMoved: card.controller.change("backgroundOpacity", value)
            }
        }
    }
}
