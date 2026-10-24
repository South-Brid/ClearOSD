import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "components"

ApplicationWindow {
    id: settings
    width: 1000; height: 780
    minimumWidth: 880; minimumHeight: 620
    visible: true
    title: "ClearOSD · 监控与亮度调节"
    color: Theme.window
    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontBody
    palette.window: Theme.window
    palette.windowText: Theme.text
    palette.base: Theme.control
    palette.alternateBase: Theme.surface
    palette.text: Theme.text
    palette.button: Theme.control
    palette.buttonText: Theme.text
    palette.highlight: Theme.accent
    palette.highlightedText: Theme.accentForeground
    palette.toolTipBase: Theme.surfaceRaised
    palette.toolTipText: Theme.text
    palette.placeholderText: Theme.textMuted
    palette.mid: Theme.controlPressed
    palette.light: Theme.controlHover
    palette.dark: Theme.border

    property int page: 0

    onClosing: function(close) { close.accepted = false; hide() }
    Connections {
        target: backend
        function onShowSettings() { settings.show(); settings.raise(); settings.requestActivate() }
    }
    ColorDialog {
        id: colorPicker
        property string field: "textColor"
        onAccepted: backend.change(field, selectedColor.toString())
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 236
            color: Theme.sidebar

            Rectangle {
                anchors.right: parent.right
                width: 1
                height: parent.height
                color: Theme.border
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                anchors.rightMargin: 17
                spacing: 6

                RowLayout {
                    Layout.topMargin: 10
                    Layout.leftMargin: 4
                    Layout.bottomMargin: 22
                    spacing: 12
                    Rectangle {
                        implicitWidth: 40
                        implicitHeight: 40
                        radius: 12
                        color: "#141d2b"
                        border.color: Theme.accentBorder
                        Label {
                            anchors.centerIn: parent
                            text: "O"
                            color: "#8ee3b2"
                            font.family: "Segoe UI"
                            font.pixelSize: 21
                            font.bold: true
                        }
                    }
                    ColumnLayout {
                        spacing: 1
                        Label {
                            text: "ClearOSD"
                            color: Theme.text
                            font.pixelSize: 17
                            font.bold: true
                        }
                        Label {
                            text: "监控浮层 · 亮度调节"
                            color: Theme.textMuted
                            font.pixelSize: Theme.fontTiny
                        }
                    }
                }

                NavItem {
                    Layout.fillWidth: true
                    glyph: Theme.iconMonitor
                    text: "硬件监控"
                    caption: "浮层内容、外观与位置"
                    checked: settings.page === 0
                    onClicked: settings.page = 0
                }
                NavItem {
                    Layout.fillWidth: true
                    glyph: Theme.iconBrightness
                    text: "亮度调节"
                    caption: "Gamma、方案与快捷键"
                    checked: settings.page === 1
                    onClicked: settings.page = 1
                }

                Item { Layout.fillHeight: true }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: quickPanel.implicitHeight + 28
                    radius: 12
                    color: Theme.surface
                    border.color: Theme.border

                    ColumnLayout {
                        id: quickPanel
                        x: 14
                        y: 14
                        width: parent.width - 28
                        spacing: 8
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            Rectangle {
                                implicitWidth: 8
                                implicitHeight: 8
                                radius: 4
                                color: backend.desktopVisible ? Theme.accent : Theme.textMuted
                            }
                            Label {
                                Layout.fillWidth: true
                                text: "桌面浮层"
                                color: Theme.text
                                font.pixelSize: Theme.fontBody
                                font.weight: Font.Medium
                            }
                            UiSwitch {
                                checked: backend.desktopVisible
                                onToggled: backend.toggleHidden()
                            }
                        }
                        Label {
                            Layout.fillWidth: true
                            text: !backend.desktopVisible ? "已隐藏"
                                  : backend.config.locked ? "已锁定 · 鼠标穿透" : "可拖动 · 右键打开设置"
                            color: Theme.textMuted
                            font.pixelSize: Theme.fontTiny
                            wrapMode: Text.WordWrap
                        }
                    }
                }

                UiButton {
                    Layout.fillWidth: true
                    Layout.topMargin: 4
                    variant: "ghost"
                    glyph: Theme.iconMinimize
                    text: "收起到托盘"
                    onClicked: settings.hide()
                }
                Caption {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.bottomMargin: 2
                    text: "设置自动保存"
                    font.pixelSize: Theme.fontTiny
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.window

            StackLayout {
                anchors.fill: parent
                currentIndex: settings.page

                MonitorPage {
                    controller: backend
                    onPickColor: function(field, value) { colorPicker.field = field; colorPicker.selectedColor = value; colorPicker.open() }
                }
                BrightnessPage { controller: brightnessBackend }
            }
        }
    }

    OverlayWindow { controller: backend }
}
