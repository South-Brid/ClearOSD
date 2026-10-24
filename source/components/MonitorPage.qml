import QtQuick
import QtQuick.Layouts
import "."

PageScroll {
    id: page
    required property var controller
    signal pickColor(string field, color value)

    title: "硬件监控"
    subtitle: "选择浮层显示的数据、外观，以及它在桌面上的位置"
    headerActions: StatusPill {
        text: page.controller.status
        warning: page.controller.status.indexOf("未连接") >= 0 || page.controller.status.indexOf("—") >= 0
    }

    PreviewCard { controller: page.controller }
    MetricsCard { controller: page.controller }
    AppearanceCard {
        controller: page.controller
        onPickColor: function(field, value) { page.pickColor(field, value) }
    }
    PositionCard { controller: page.controller }
}
