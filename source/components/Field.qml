import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

// Label above a control, with an optional value shown on the right.
ColumnLayout {
    id: field

    property string label
    property string value
    default property alias control: holder.data

    spacing: 8

    RowLayout {
        Layout.fillWidth: true
        Label {
            Layout.fillWidth: true
            text: field.label
            color: Theme.textSecondary
            font.pixelSize: Theme.fontCaption
        }
        Label {
            visible: field.value.length > 0
            text: field.value
            color: Theme.text
            font.pixelSize: Theme.fontCaption
            font.weight: Font.DemiBold
        }
    }

    RowLayout {
        id: holder
        Layout.fillWidth: true
        spacing: 8
    }
}
