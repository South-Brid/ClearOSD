#pragma once
#include <QStringList>
namespace MetricCatalog {
inline const QStringList keys={"fps","trueFps","gpu","cpu","ram","cpuPower","ramPower","gpuPower","gpuTemp","vram","cpuTemp"};
inline const QStringList titles={QStringLiteral("帧率（RTSS）"),QStringLiteral("真实帧率（True FPS）"),QStringLiteral("GPU 使用率"),QStringLiteral("CPU 使用率"),QStringLiteral("内存使用率"),QStringLiteral("CPU 功耗"),QStringLiteral("内存功耗"),QStringLiteral("GPU 功耗"),QStringLiteral("GPU 温度"),QStringLiteral("显存占用"),QStringLiteral("CPU 温度")};
inline const QStringList labels={"FPS","True FPS","GPU","CPU","RAM","CPU","RAM","GPU","GPU","VRAM","CPU"};
inline const QStringList units={"","","%","%","%","W","W","W",QStringLiteral("°C"),"GB",QStringLiteral("°C")};

}
