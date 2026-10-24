import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

AbstractButton {
    id: tile

    property bool selected: false
    property bool available: true

    implicitHeight: 42
    hoverEnabled: true
    leftPadding: 12
    rightPadding: 12

    contentItem: RowLayout {
        spacing: 10
        Rectangle {
            implicitWidth: 18
            implicitHeight: 18
            radius: 5
            color: tile.selected ? Theme.accent : "transparent"
            border.width: tile.selected ? 0 : 1.5
            border.color: tile.hovered ? Theme.textMuted : Theme.borderStrong
            Icon {
                anchors.centerIn: parent
                visible: tile.selected
                glyph: Theme.iconCheck
                size: 11
                color: Theme.accentForeground
            }
        }
        Label {
            Layout.fillWidth: true
            text: tile.text
            color: tile.selected ? Theme.text : Theme.textSecondary
            font.pixelSize: Theme.fontBody
            elide: Text.ElideRight
        }
        Label {
            visible: !tile.available
            text: "无数据"
            color: Theme.textMuted
            font.pixelSize: Theme.fontTiny
        }
    }

    background: Rectangle {
        radius: Theme.radiusMedium
        color: tile.selected ? Theme.accentSoft : tile.hovered ? Theme.controlHover : Theme.control
        border.width: tile.visualFocus ? 2 : 1
        border.color: tile.visualFocus || tile.selected ? Theme.accentBorder : Theme.border
        Behavior on color { ColorAnimation { duration: 110 } }
    }
}
