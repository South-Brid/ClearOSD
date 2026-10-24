import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

ScrollView {
    id: view

    property string title
    property string subtitle
    default property alias content: column.data
    property alias headerActions: actionRow.data

    clip: true
    contentWidth: availableWidth
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical: ScrollBar {
        parent: view
        x: view.width - width - 3
        y: 6
        height: view.height - 12
        contentItem: Rectangle {
            implicitWidth: 6
            radius: 3
            color: parent.pressed ? Theme.textMuted : Theme.borderStrong
            opacity: parent.policy === ScrollBar.AlwaysOn || (parent.active && parent.size < 1.0) ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: 200 } }
        }
    }

    Item {
        width: view.availableWidth
        implicitHeight: column.implicitHeight + 64

        ColumnLayout {
            id: column
            width: Math.min(view.availableWidth - 64, 860)
            x: (view.availableWidth - width) / 2
            y: 30
            spacing: 16

            RowLayout {
                Layout.fillWidth: true
                Layout.bottomMargin: 4
                spacing: 16
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Label {
                        text: view.title
                        color: Theme.text
                        font.pixelSize: Theme.fontTitle
                        font.weight: Font.Bold
                    }
                    Label {
                        Layout.fillWidth: true
                        text: view.subtitle
                        color: Theme.textMuted
                        font.pixelSize: Theme.fontBody
                        wrapMode: Text.WordWrap
                    }
                }
                RowLayout {
                    id: actionRow
                    spacing: 10
                }
            }
        }
    }
}
