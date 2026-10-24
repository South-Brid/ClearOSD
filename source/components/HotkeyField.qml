import QtQuick
import QtQuick.Controls
import "."

// Records a key combination instead of accepting typed text.
UiTextField {
    id: field

    readOnly: true
    placeholderText: activeFocus ? "按下组合键，例如 Ctrl+Alt+1" : "点击后按下快捷键"
    leftPadding: 36

    Icon {
        x: 12
        anchors.verticalCenter: parent.verticalCenter
        glyph: Theme.iconKeyboard
        size: 14
        color: field.activeFocus ? Theme.accent : Theme.textMuted
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab)
            return
        event.accepted = true
        if (event.key === Qt.Key_Escape) {
            field.focus = false
            return
        }
        if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
            field.text = ""
            return
        }
        let key = ""
        if (event.key >= Qt.Key_A && event.key <= Qt.Key_Z)
            key = String.fromCharCode(event.key)
        else if (event.key >= Qt.Key_0 && event.key <= Qt.Key_9)
            key = String.fromCharCode(event.key)
        else if (event.key >= Qt.Key_F1 && event.key <= Qt.Key_F24)
            key = "F" + (event.key - Qt.Key_F1 + 1)
        const parts = []
        if (event.modifiers & Qt.ControlModifier) parts.push("Ctrl")
        if (event.modifiers & Qt.AltModifier) parts.push("Alt")
        if (event.modifiers & Qt.ShiftModifier) parts.push("Shift")
        if (event.modifiers & Qt.MetaModifier) parts.push("Win")
        if (key.length > 0 && parts.length > 0)
            field.text = parts.concat([key]).join("+")
    }
}
