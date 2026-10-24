import QtQuick
import QtQuick.Controls
import "."

// A miniature monitor whose 3x3 cells select where the overlay is anchored.
Item {
    id: root

    property string anchor: "topRight"
    property bool freeMode: false
    signal anchorPicked(string key)

    readonly property var keys: ["topLeft", "topCenter", "topRight",
                                 "centerLeft", "center", "centerRight",
                                 "bottomLeft", "bottomCenter", "bottomRight"]
    readonly property var names: ["左上角", "顶部居中", "右上角",
                                  "左侧居中", "屏幕中央", "右侧居中",
                                  "左下角", "底部居中", "右下角"]

    implicitWidth: 288
    implicitHeight: bezel.height + 22

    Rectangle {
        id: bezel
        width: root.width
        height: Math.round(width * 10 / 16)
        radius: 12
        color: "#080b10"
        border.color: Theme.borderStrong

        Rectangle {
            id: screen
            anchors.fill: parent
            anchors.margins: 7
            radius: 7
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#1b2b3d" }
                GradientStop { position: 1.0; color: "#11211c" }
            }

            Grid {
                id: grid
                anchors.fill: parent
                anchors.margins: 6
                columns: 3
                spacing: 5

                Repeater {
                    model: 9
                    delegate: Rectangle {
                        id: cell
                        required property int index
                        readonly property int row: Math.floor(index / 3)
                        readonly property int column: index % 3
                        readonly property bool selected: !root.freeMode && root.keys[index] === root.anchor

                        width: (grid.width - 2 * grid.spacing) / 3
                        height: (grid.height - 2 * grid.spacing) / 3
                        radius: 5
                        color: selected ? "#2e5bd6a0" : area.containsMouse ? "#14ffffff" : "transparent"
                        border.width: selected ? 1.5 : 1
                        border.color: selected ? Theme.accent : area.containsMouse ? "#45ffffff" : "#12ffffff"
                        Behavior on color { ColorAnimation { duration: 110 } }

                        Rectangle {
                            width: Math.min(parent.width * 0.62, 56)
                            height: 8
                            radius: 3
                            x: cell.column === 0 ? 6 : cell.column === 2 ? parent.width - width - 6 : (parent.width - width) / 2
                            y: cell.row === 0 ? 6 : cell.row === 2 ? parent.height - height - 6 : (parent.height - height) / 2
                            color: cell.selected ? Theme.accent : Theme.textSecondary
                            opacity: cell.selected ? 1 : area.containsMouse ? 0.65 : 0.16
                        }

                        MouseArea {
                            id: area
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.anchorPicked(root.keys[cell.index])
                        }

                        ToolTip.visible: area.containsMouse
                        ToolTip.delay: 350
                        ToolTip.text: root.names[index]
                    }
                }
            }

            Rectangle {
                visible: root.freeMode
                anchors.centerIn: parent
                implicitWidth: freeLabel.implicitWidth + 24
                implicitHeight: 30
                radius: 15
                color: "#e6141a23"
                border.color: Theme.accentBorder
                Label {
                    id: freeLabel
                    anchors.centerIn: parent
                    text: "自由拖动中 · 点击格子切回固定"
                    color: Theme.accent
                    font.pixelSize: Theme.fontCaption
                }
            }
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: bezel.height
        width: 34
        height: 14
        color: "#10151c"
    }
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: bezel.height + 13
        width: 104
        height: 6
        radius: 3
        color: Theme.borderStrong
    }
}
