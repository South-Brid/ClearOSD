import QtQuick
import "."

Text {
    property string glyph
    property real size: 16
    text: glyph
    color: Theme.textSecondary
    font.family: Theme.iconFamily
    font.pixelSize: size
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
}
