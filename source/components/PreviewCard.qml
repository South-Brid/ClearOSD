import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

SectionCard {
    id: card
    required property var controller

    title: "实时预览"
    subtitle: controller.desktopVisible ? "浮层正在桌面上显示" : "浮层已隐藏，预览仍会刷新"
    glyph: Theme.iconEye
    actions: UiSwitch {
        text: "显示浮层"
        checked: card.controller.desktopVisible
        onToggled: card.controller.toggleHidden()
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 104
        radius: 12
        clip: true
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#1a2b3e" }
            GradientStop { position: 0.55; color: "#162432" }
            GradientStop { position: 1.0; color: "#12261f" }
        }
        border.color: Theme.border

        Flickable {
            id: flick
            anchors.fill: parent
            anchors.margins: 14
            clip: true
            contentWidth: Math.max(width, strip.width)
            contentHeight: height
            interactive: strip.width > width
            boundsBehavior: Flickable.StopAtBounds

            MetricStrip {
                id: strip
                x: Math.max(0, (flick.width - width) / 2)
                y: (flick.height - height) / 2
                presentation: card.controller.overlayStyle
                pixelRatio: Screen.devicePixelRatio
            }
        }
    }
}
