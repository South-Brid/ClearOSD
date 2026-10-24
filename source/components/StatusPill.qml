import QtQuick
import QtQuick.Controls
import "."

Rectangle {
    id: pill

    property string text
    property bool warning: false

    implicitHeight: 30
    implicitWidth: Math.min(label.implicitWidth + 44, 340)
    radius: 15
    color: Theme.surface
    border.color: Theme.border

    Row {
        id: row
        anchors.verticalCenter: parent.verticalCenter
        x: 12
        spacing: 8
        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: 8
            height: 8
            radius: 4
            color: pill.warning ? Theme.warning : Theme.accent
        }
        Label {
            id: label
            anchors.verticalCenter: parent.verticalCenter
            width: pill.width - 44
            text: pill.text
            color: Theme.textSecondary
            font.pixelSize: Theme.fontCaption
            elide: Text.ElideRight
        }
    }
}
