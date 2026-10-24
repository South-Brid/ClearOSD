import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

PageScroll {
    id: page
    required property var controller
    readonly property bool hasDisplays: controller.monitors.length > 0
    property bool confirmDelete: false

    title: "亮度调节"
    subtitle: "通过 Gamma 曲线调节显示效果；临时调节在退出时自动恢复，需要保留请保存为方案"
    headerActions: UiSwitch {
        text: "开机启动"
        checked: page.controller.startupEnabled
        onToggled: page.controller.startupEnabled = checked
    }

    Connections {
        target: page.controller
        function onSelectedProfileChanged() {
            profileName.text = page.controller.selectedProfile
            page.confirmDelete = false
        }
        function onSelectedHotkeyChanged() { hotkey.text = page.controller.selectedHotkey }
    }
    Timer {
        id: confirmTimeout
        interval: 3000
        onTriggered: page.confirmDelete = false
    }

    SectionCard {
        title: "显示器"
        subtitle: page.controller.monitors.length > 1 ? "选择要调节的显示器" : "当前只检测到一台可调节的显示器"
        glyph: Theme.iconDisplay

        GridLayout {
            Layout.fillWidth: true
            visible: page.hasDisplays
            columns: Math.max(1, Math.min(page.controller.monitors.length, 3))
            columnSpacing: 10
            rowSpacing: 10
            Repeater {
                model: page.controller.monitors
                delegate: MonitorTile {
                    required property var modelData
                    required property int index
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    text: "显示器 " + (index + 1)
                    detail: modelData.width + " × " + modelData.height
                    primary: modelData.primary
                    selected: index === page.controller.currentMonitorIndex
                    onClicked: page.controller.currentMonitorIndex = index
                }
            }
        }
        Caption {
            visible: !page.hasDisplays
            text: "未检测到可调节的显示器"
        }
    }

    SectionCard {
        title: "显示调节"
        subtitle: "拖动滑杆或直接输入数值，更改实时生效"
        glyph: Theme.iconBrightness
        enabled: page.hasDisplays
        actions: UiButton {
            variant: "ghost"
            glyph: Theme.iconReset
            text: "恢复当前"
            onClicked: page.controller.resetCurrent()
        }

        GammaSlider {
            Layout.fillWidth: true
            label: "Gamma"
            from: 0.2; to: 3.0; stepSize: 0.01; decimals: 2; defaultValue: 1.0
            currentValue: page.controller.gamma
            onChangeRequested: value => page.controller.gamma = value
            onResetRequested: page.controller.resetParameter("gamma")
        }
        GammaSlider {
            Layout.fillWidth: true
            label: "亮度"
            from: -128; to: 128; stepSize: 1; decimals: 0; defaultValue: 0
            currentValue: page.controller.brightness
            onChangeRequested: value => page.controller.brightness = value
            onResetRequested: page.controller.resetParameter("brightness")
        }
        GammaSlider {
            Layout.fillWidth: true
            label: "对比度"
            from: 0.1; to: 3.0; stepSize: 0.01; decimals: 2; defaultValue: 1.0
            currentValue: page.controller.contrast
            onChangeRequested: value => page.controller.contrast = value
            onResetRequested: page.controller.resetParameter("contrast")
        }

        Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Theme.border }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            UiButton {
                glyph: Theme.iconReset
                text: "恢复全部显示器"
                onClicked: page.controller.resetAll()
            }
            Item { Layout.fillWidth: true }
            UiButton {
                variant: "primary"
                glyph: Theme.iconCopy
                text: "同步到全部显示器"
                enabled: page.controller.monitors.length > 1
                onClicked: page.controller.syncAll()
            }
        }
    }

    SectionCard {
        title: "方案与快捷键"
        subtitle: "点击方案立即应用到当前显示器；保存会记录当前显示器的参数"
        glyph: Theme.iconProfile

        Flow {
            Layout.fillWidth: true
            spacing: 8
            Repeater {
                model: page.controller.profiles
                delegate: ProfileChip {
                    required property string modelData
                    text: modelData
                    hotkey: page.controller.profileHotkeys[modelData] || ""
                    selected: modelData === page.controller.selectedProfile
                    enabled: page.hasDisplays
                    onClicked: page.controller.selectProfile(modelData)
                }
            }
        }

        Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Theme.border }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 16
            rowSpacing: 14

            Field {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                label: "方案名称"
                UiTextField {
                    id: profileName
                    Layout.fillWidth: true
                    placeholderText: "输入新名称即可另存为新方案"
                    text: page.controller.selectedProfile
                }
            }
            Field {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                label: "快捷键（可选，Backspace 清除）"
                HotkeyField {
                    id: hotkey
                    Layout.fillWidth: true
                    text: page.controller.selectedHotkey
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            UiButton {
                variant: "primary"
                glyph: Theme.iconSave
                text: "保存当前参数"
                enabled: page.hasDisplays && profileName.text.trim().length > 0
                onClicked: page.controller.saveProfile(profileName.text, hotkey.text)
            }
            UiButton {
                glyph: Theme.iconUnlink
                text: "解绑热键"
                enabled: page.controller.selectedHotkey.length > 0
                onClicked: page.controller.unbindHotkey(page.controller.selectedProfile)
            }
            Item { Layout.fillWidth: true }
            UiButton {
                variant: "danger"
                glyph: Theme.iconDelete
                text: page.confirmDelete ? "再次点击确认删除" : "删除“" + page.controller.selectedProfile + "”"
                enabled: page.controller.selectedProfile !== "default"
                onClicked: {
                    if (!page.confirmDelete) {
                        page.confirmDelete = true
                        confirmTimeout.restart()
                        return
                    }
                    page.confirmDelete = false
                    page.controller.deleteProfile(page.controller.selectedProfile)
                }
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: statusRow.implicitHeight + 20
        radius: Theme.radiusMedium
        color: Theme.surface
        border.color: Theme.border
        RowLayout {
            id: statusRow
            x: 14
            y: 10
            width: parent.width - 28
            spacing: 10
            Icon {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 2
                glyph: Theme.iconInfo
                size: 14
                color: Theme.accent
            }
            Label {
                Layout.fillWidth: true
                text: page.controller.statusText
                color: Theme.textSecondary
                font.pixelSize: Theme.fontCaption
                wrapMode: Text.WordWrap
            }
        }
    }
}
