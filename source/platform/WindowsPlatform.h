#pragma once
#include <windows.h>
#include <QString>
namespace WindowsPlatform {
QString dataDir();
QString rtssDir();
DWORD processId(const QString &name);
DWORD foregroundProcessId();
bool launchHidden(const QString &path,const QString &args={});
}
