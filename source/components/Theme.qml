pragma Singleton
import QtQuick

QtObject {
    readonly property string fontFamily: "Microsoft YaHei UI"
    // Segoe Fluent Icons ships with Windows 11; MDL2 Assets shares its glyph codes on Windows 10.
    readonly property string iconFamily: Qt.fontFamilies().indexOf("Segoe Fluent Icons") >= 0
                                         ? "Segoe Fluent Icons" : "Segoe MDL2 Assets"

    readonly property color window: "#0b0f15"
    readonly property color sidebar: "#0f141b"
    readonly property color surface: "#141a23"
    readonly property color surfaceRaised: "#19202b"
    readonly property color control: "#1c2430"
    readonly property color controlHover: "#232c3a"
    readonly property color controlPressed: "#2a3444"
    readonly property color border: "#222a36"
    readonly property color borderStrong: "#313b4b"
    readonly property color track: "#2a3342"

    readonly property color text: "#e9eef5"
    readonly property color textSecondary: "#aab4c3"
    readonly property color textMuted: "#78849a"
    readonly property color textDisabled: "#4f5a6b"

    readonly property color accent: "#5bd6a0"
    readonly property color accentHover: "#74e0b1"
    readonly property color accentPressed: "#47bf8b"
    readonly property color accentSoft: "#1f5bd6a0"
    readonly property color accentBorder: "#805bd6a0"
    readonly property color accentForeground: "#04211a"

    readonly property color warning: "#f0b86e"
    readonly property color danger: "#f2777a"
    readonly property color dangerSoft: "#1ff2777a"

    readonly property int radiusLarge: 16
    readonly property int radiusMedium: 10
    readonly property int radiusSmall: 6
    readonly property int controlHeight: 36

    readonly property int fontTitle: 22
    readonly property int fontSection: 15
    readonly property int fontBody: 13
    readonly property int fontCaption: 12
    readonly property int fontTiny: 11

    // "ctrl+alt+1" -> "Ctrl+Alt+1"; stored hotkeys may come from older versions.
    function formatHotkey(sequence) {
        return String(sequence || "").split("+").map(function(part) {
            part = part.trim()
            return part.length > 0 ? part.charAt(0).toUpperCase() + part.slice(1) : part
        }).join("+")
    }

    // Segoe Fluent Icons / MDL2 code points.
    readonly property string iconMonitor: "\uE9D9"
    readonly property string iconBrightness: "\uE706"
    readonly property string iconDisplay: "\uE7F4"
    readonly property string iconEye: "\uE890"
    readonly property string iconList: "\uE8FD"
    readonly property string iconColor: "\uE790"
    readonly property string iconPosition: "\uE81E"
    readonly property string iconLock: "\uE72E"
    readonly property string iconInfo: "\uE946"
    readonly property string iconCheck: "\uE73E"
    readonly property string iconChevronDown: "\uE70D"
    readonly property string iconReset: "\uE777"
    readonly property string iconCopy: "\uE8C8"
    readonly property string iconSave: "\uE74E"
    readonly property string iconDelete: "\uE74D"
    readonly property string iconKeyboard: "\uE765"
    readonly property string iconUnlink: "\uE711"
    readonly property string iconMinimize: "\uE921"
    readonly property string iconAdd: "\uE710"
    readonly property string iconRemove: "\uE738"
    readonly property string iconEdit: "\uE70F"
    readonly property string iconProfile: "\uE8F1"
}
