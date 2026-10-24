#include "brightness/services/StartupManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QProcess>
#include <windows.h>

namespace {

constexpr auto registryPath =
    "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr auto valueName = "ClearOSD";

bool scheduledTask(const QStringList &arguments)
{
    QProcess process;
    process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args) {
        args->flags |= CREATE_NO_WINDOW;
    });
    process.start(QStringLiteral("schtasks.exe"), arguments);
    if(!process.waitForFinished(10000)){process.kill();process.waitForFinished();return false;}
    return process.exitStatus()==QProcess::NormalExit && process.exitCode()==0;
}

QSettings startupSettings()
{
    return QSettings(QString::fromLatin1(registryPath), QSettings::NativeFormat);
}

} // namespace

bool StartupManager::isEnabled()
{
    return scheduledTask({"/Query","/TN","ClearOSD"});
}

bool StartupManager::setEnabled(bool enabled)
{
    if(!enabled)return !isEnabled() || scheduledTask({"/Delete","/TN","ClearOSD","/F"});
    const auto executable=QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    // ClearOSD already requires elevation for RTSS; use one logon task.
    return scheduledTask({"/Create","/TN","ClearOSD","/SC","ONLOGON","/RL","HIGHEST",
        "/TR",QStringLiteral("\"%1\" --background").arg(executable),"/F"});
}
