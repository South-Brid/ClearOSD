import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

Rectangle {
    id: card

    property string title
    property string subtitle
    property string glyph
    property int padding: 20
    default property alias content: body.data
    property alias actions: actionRow.data

    Layout.fillWidth: true
    implicitHeight: column.implicitHeight + 2 * padding
    radius: Theme.radiusLarge
    color: Theme.surface
    border.color: Theme.border

    ColumnLayout {
        id: column
        x: card.padding
        y: card.padding
        width: card.width - 2 * card.padding
        spacing: 18

        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            visible: card.title.length > 0

            Rectangle {
                visible: card.glyph.length > 0
                implicitWidth: 34
                implicitHeight: 34
                radius: 10
                color: Theme.accentSoft
                Icon {
                    anchors.centerIn: parent
                    glyph: card.glyph
                    size: 16
                    color: Theme.accent
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Label {
                    text: card.title
                    color: Theme.text
                    font.pixelSize: Theme.fontSection
                    font.weight: Font.DemiBold
                }
                Label {
                    Layout.fillWidth: true
                    visible: card.subtitle.length > 0
                    text: card.subtitle
                    color: Theme.textMuted
                    font.pixelSize: Theme.fontCaption
                    wrapMode: Text.WordWrap
                }
            }

            RowLayout {
                id: actionRow
                spacing: 8
            }
        }

        ColumnLayout {
            id: body
            Layout.fillWidth: true
            spacing: 14
        }
    }
}
