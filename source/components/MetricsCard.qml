import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

SectionCard {
    id: card
    required property var controller

    title: "显示项目"
    subtitle: "勾选要在浮层中显示的数据，至少保留一项"
    glyph: Theme.iconList

    GridLayout {
        Layout.fillWidth: true
        columns: width > 560 ? 3 : 2
        columnSpacing: 10
        rowSpacing: 10

        Repeater {
            // A numeric model keeps delegates (and their hover state) alive
            // while the option list is rebuilt on every sample.
            model: card.controller.metricOptions.length
            delegate: MetricTile {
                required property int index
                readonly property var option: card.controller.metricOptions[index]
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                text: option ? option.title : ""
                selected: option ? option.enabled : false
                available: option ? option.available : true
                onClicked: card.controller.toggleMetric(option.key, !option.enabled)
            }
        }
    }

    Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Theme.border }

    RowLayout {
        Layout.fillWidth: true
        spacing: 12
        Label {
            text: "刷新间隔"
            color: Theme.text
            font.pixelSize: Theme.fontBody
        }
        UiStepper {
            from: 100
            to: 10000
            stepSize: 100
            value: card.controller.config.refreshIntervalMs
            onValueModified: card.controller.change("refreshIntervalMs", value)
        }
        Caption {
            Layout.fillWidth: true
            text: "毫秒 · 每秒 " + (1000 / card.controller.config.refreshIntervalMs).toFixed(1) + " 次"
        }
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: note.implicitHeight + 22
        radius: Theme.radiusMedium
        color: Theme.window
        border.color: Theme.border

        RowLayout {
            id: note
            x: 14
            y: 11
            width: parent.width - 28
            spacing: 10
            Icon {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 2
                glyph: Theme.iconInfo
                size: 14
                color: Theme.textMuted
            }
            Label {
                Layout.fillWidth: true
                text: card.controller.sensorNote
                color: Theme.textMuted
                font.pixelSize: Theme.fontCaption
                wrapMode: Text.WordWrap
                lineHeight: 1.25
            }
        }
    }
}
