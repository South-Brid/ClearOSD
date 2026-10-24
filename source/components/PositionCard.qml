import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

SectionCard {
    id: card
    required property var controller

    readonly property string currentAnchor: controller.config.anchor
    readonly property bool freeMode: currentAnchor === "custom"
    property string lastFixedAnchor: "topRight"
    onCurrentAnchorChanged: if (currentAnchor !== "custom") lastFixedAnchor = currentAnchor
    Component.onCompleted: if (currentAnchor !== "custom") lastFixedAnchor = currentAnchor

    title: "位置"
    subtitle: "九宫格快速定位，或解锁后在桌面上自由拖动"
    glyph: Theme.iconPosition

    RowLayout {
        Layout.fillWidth: true
        spacing: 28

        PositionGrid {
            Layout.preferredWidth: 288
            Layout.alignment: Qt.AlignTop
            anchor: card.currentAnchor
            freeMode: card.freeMode
            onAnchorPicked: key => card.controller.change("anchor", key)
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 16

            Field {
                Layout.fillWidth: true
                label: "定位方式"
                SegmentedControl {
                    Layout.fillWidth: true
                    options: ["固定位置", "自由拖动"]
                    currentIndex: card.freeMode ? 1 : 0
                    onActivated: index => card.controller.change("anchor", index === 1 ? "custom" : card.lastFixedAnchor)
                }
            }

            Field {
                Layout.fillWidth: true
                label: "显示屏幕"
                UiComboBox {
                    Layout.fillWidth: true
                    model: card.controller.screenNames
                    currentIndex: card.controller.config.screen
                    onActivated: card.controller.change("screen", currentIndex)
                }
            }

            Field {
                Layout.fillWidth: true
                label: "边缘距离"
                value: card.controller.config.edgeMargin + " px"
                enabled: !card.freeMode
                opacity: enabled ? 1 : 0.45
                UiSlider {
                    Layout.fillWidth: true
                    from: 0
                    to: 120
                    stepSize: 1
                    value: card.controller.config.edgeMargin
                    onMoved: card.controller.change("edgeMargin", Math.round(value))
                }
            }
        }
    }

    Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Theme.border }

    RowLayout {
        Layout.fillWidth: true
        spacing: 12
        Icon {
            glyph: Theme.iconLock
            size: 15
            color: card.controller.config.locked ? Theme.accent : Theme.textMuted
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            Label {
                text: "锁定位置并开启鼠标穿透"
                color: Theme.text
                font.pixelSize: Theme.fontBody
            }
            Caption {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: card.controller.config.locked
                      ? "浮层不响应鼠标，不会挡住游戏操作；可从系统托盘解锁。"
                      : "可直接拖动浮层（会切换为自由拖动），右键浮层打开设置。"
            }
        }
        UiSwitch {
            checked: card.controller.config.locked
            onToggled: card.controller.change("locked", checked)
        }
    }
}
