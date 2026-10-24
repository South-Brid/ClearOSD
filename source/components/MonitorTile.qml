import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

AbstractButton {
    id: tile

    property string detail
    property bool primary: false
    property bool selected: false

    implicitHeight: 64
    hoverEnabled: true
    padding: 12

    contentItem: RowLayout {
        spacing: 12
        Rectangle {
            implicitWidth: 40
            implicitHeight: 40
            radius: 10
            color: tile.selected ? Theme.accent : Theme.surfaceRaised
            Icon {
                anchors.centerIn: parent
                glyph: Theme.iconDisplay
                size: 18
                color: tile.selected ? Theme.accentForeground : Theme.textSecondary
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3
            RowLayout {
                Layout.fillWidth: true
                spacing: 6
                Label {
                    text: tile.text
                    color: Theme.text
                    font.pixelSize: Theme.fontBody
                    font.weight: Font.DemiBold
                }
                Rectangle {
                    visible: tile.primary
                    implicitWidth: primaryLabel.implicitWidth + 10
                    implicitHeight: 18
                    radius: 5
                    color: Theme.accentSoft
                    Label {
                        id: primaryLabel
                        anchors.centerIn: parent
                        text: "主屏"
                        color: Theme.accent
                        font.pixelSize: Theme.fontTiny
                    }
                }
                Item { Layout.fillWidth: true }
            }
            Label {
                Layout.fillWidth: true
                text: tile.detail
                color: Theme.textMuted
                font.pixelSize: Theme.fontCaption
            }
        }
    }

    background: Rectangle {
        radius: 12
        color: tile.selected ? Theme.accentSoft : tile.hovered ? Theme.controlHover : Theme.control
        border.width: tile.selected || tile.visualFocus ? 1.5 : 1
        border.color: tile.selected || tile.visualFocus ? Theme.accent : Theme.border
        Behavior on color { ColorAnimation { duration: 110 } }
    }
}
