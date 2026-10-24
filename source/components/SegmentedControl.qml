import QtQuick
import QtQuick.Controls
import "."

Rectangle {
    id: root

    property var options: []
    property int currentIndex: 0
    signal activated(int index)

    implicitHeight: Theme.controlHeight
    implicitWidth: 240
    radius: Theme.radiusMedium
    color: Theme.control
    border.color: Theme.border

    readonly property real segmentWidth: (width - 6) / Math.max(1, options.length)

    Rectangle {
        x: 3 + root.currentIndex * root.segmentWidth
        y: 3
        width: root.segmentWidth
        height: root.height - 6
        radius: Theme.radiusMedium - 3
        color: Theme.surfaceRaised
        border.color: Theme.borderStrong
        Behavior on x { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }
    }

    Row {
        x: 3
        y: 3
        Repeater {
            model: root.options
            delegate: AbstractButton {
                id: segment
                required property int index
                required property var modelData
                width: root.segmentWidth
                height: root.height - 6
                hoverEnabled: true
                onClicked: root.activated(index)
                contentItem: Label {
                    text: segment.modelData
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: Theme.fontBody
                    font.weight: root.currentIndex === segment.index ? Font.DemiBold : Font.Normal
                    color: root.currentIndex === segment.index ? Theme.text
                         : segment.hovered ? Theme.textSecondary : Theme.textMuted
                }
                background: Item { }
            }
        }
    }
}
