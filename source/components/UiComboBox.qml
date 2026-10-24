import QtQuick
import QtQuick.Controls
import "."

ComboBox {
    id: control

    implicitHeight: Theme.controlHeight
    leftPadding: 12
    rightPadding: 36
    hoverEnabled: true
    font.pixelSize: Theme.fontBody

    contentItem: Label {
        text: control.displayText
        font: control.font
        color: Theme.text
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Icon {
        x: control.width - width - 12
        y: (control.height - height) / 2
        glyph: Theme.iconChevronDown
        size: 11
        color: control.popup.visible ? Theme.accent : Theme.textMuted
        rotation: control.popup.visible ? 180 : 0
        Behavior on rotation { NumberAnimation { duration: 140 } }
    }

    background: Rectangle {
        implicitWidth: 180
        radius: Theme.radiusMedium
        color: control.down ? Theme.controlPressed : control.hovered ? Theme.controlHover : Theme.control
        border.width: control.visualFocus || control.popup.visible ? 2 : 1
        border.color: control.visualFocus || control.popup.visible ? Theme.accent : Theme.border
    }

    delegate: ItemDelegate {
        id: item
        required property int index
        width: ListView.view ? ListView.view.width : control.width
        implicitHeight: 34
        highlighted: control.highlightedIndex === index
        leftPadding: 10
        contentItem: Label {
            text: control.textAt(item.index)
            font: control.font
            color: control.currentIndex === item.index ? Theme.accent : Theme.text
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            radius: Theme.radiusSmall
            color: item.highlighted ? Theme.controlHover : "transparent"
        }
    }

    popup: Popup {
        y: control.height + 4
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight + 12, 280)
        padding: 6

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator { }
        }

        background: Rectangle {
            radius: Theme.radiusMedium
            color: Theme.surfaceRaised
            border.color: Theme.borderStrong
        }
    }
}
