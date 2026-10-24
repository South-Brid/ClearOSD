import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

AbstractButton {
    id: item

    property string glyph
    property string caption

    implicitHeight: 52
    hoverEnabled: true
    leftPadding: 14
    rightPadding: 12

    contentItem: RowLayout {
        spacing: 12
        Icon {
            glyph: item.glyph
            size: 17
            color: item.checked ? Theme.accent : item.hovered ? Theme.text : Theme.textMuted
            Layout.preferredWidth: 20
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 1
            Label {
                text: item.text
                color: item.checked || item.hovered ? Theme.text : Theme.textSecondary
                font.pixelSize: 14
                font.weight: item.checked ? Font.DemiBold : Font.Normal
            }
            Label {
                visible: item.caption.length > 0
                text: item.caption
                color: Theme.textMuted
                font.pixelSize: Theme.fontTiny
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }
    }

    background: Rectangle {
        radius: Theme.radiusMedium
        color: item.checked ? Theme.controlHover : item.hovered ? Theme.control : "transparent"
        border.width: item.visualFocus ? 2 : 0
        border.color: Theme.accent
        Behavior on color { ColorAnimation { duration: 120 } }

        Rectangle {
            visible: item.checked
            x: 0
            anchors.verticalCenter: parent.verticalCenter
            width: 3
            height: 20
            radius: 2
            color: Theme.accent
        }
    }
}
