#include "brightness/services/DisplayService.h"

#include <algorithm>
#include <cmath>
#include <windows.h>

namespace {

BOOL CALLBACK monitorCallback(HMONITOR monitor, HDC, LPRECT, LPARAM data)
{
    auto *displays = reinterpret_cast<QList<DisplayInfo> *>(data);

    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info)) {
        return TRUE;
    }

    const RECT &rect = info.rcMonitor;
    DisplayInfo display;
    display.deviceName = QString::fromWCharArray(info.szDevice);
    display.width = rect.right - rect.left;
    display.height = rect.bottom - rect.top;
    display.primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
    displays->append(display);
    return TRUE;
}

} // namespace

QList<DisplayInfo> DisplayService::enumerateDisplays()
{
    QList<DisplayInfo> displays;
    EnumDisplayMonitors(nullptr, nullptr, monitorCallback,
                        reinterpret_cast<LPARAM>(&displays));

    for (qsizetype index = 0; index < displays.size(); ++index) {
        DisplayInfo &display = displays[index];
        display.label = QStringLiteral("显示器 %1%2  ·  %3×%4")
                            .arg(index + 1)
                            .arg(display.primary ? QStringLiteral(" · 主屏") : QString())
                            .arg(display.width)
                            .arg(display.height);
    }
    return displays;
}

bool DisplayService::apply(const QString &deviceName,
                           const DisplayParameters &parameters)
{
    const wchar_t *device = reinterpret_cast<const wchar_t *>(deviceName.utf16());
    HDC dc = CreateDCW(device, nullptr, nullptr, nullptr);
    if (!dc) {
        return false;
    }

    WORD ramp[3][256]{};
    const double gamma = std::max(0.05, parameters.gamma);
    const double contrast = std::max(0.01, parameters.contrast);

    for (int index = 0; index < 256; ++index) {
        double value = std::pow(index / 255.0, 1.0 / gamma) * 255.0;
        value += parameters.brightness;
        value = (value - 128.0) * contrast + 128.0;
        value = std::clamp(value, 0.0, 255.0);
        const WORD word = static_cast<WORD>(std::lround(value)) * 257;
        ramp[0][index] = word;
        ramp[1][index] = word;
        ramp[2][index] = word;
    }

    const bool succeeded = SetDeviceGammaRamp(dc, ramp) != FALSE;
    DeleteDC(dc);
    return succeeded;
}

bool DisplayService::reset(const QString &deviceName)
{
    return apply(deviceName, DisplayParameters{});
}

