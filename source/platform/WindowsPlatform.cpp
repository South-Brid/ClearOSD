#include "WindowsPlatform.h"
#include <tlhelp32.h>
#include <shellapi.h>
#include <QDir>
namespace WindowsPlatform {
QString dataDir() { auto p=qEnvironmentVariable("LOCALAPPDATA")+"/ClearOSD"; QDir().mkpath(p); return p; }
QString rtssDir() { return qEnvironmentVariable("ProgramFiles(x86)")+"/RivaTuner Statistics Server"; }
DWORD foregroundProcessId() {
    DWORD pid=0;GetWindowThreadProcessId(GetForegroundWindow(),&pid);
    return pid==GetCurrentProcessId()?0:pid;
}
DWORD processId(const QString &name) {
    HANDLE s=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0); if(s==INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W e{}; e.dwSize=sizeof(e); DWORD pid=0;
    if(Process32FirstW(s,&e)) do { if(QString::fromWCharArray(e.szExeFile).compare(name,Qt::CaseInsensitive)==0){pid=e.th32ProcessID;break;} }while(Process32NextW(s,&e));
    CloseHandle(s); return pid;
}
bool launchHidden(const QString &path,const QString &args) {
    auto p=QDir::toNativeSeparators(path).toStdWString(), a=args.toStdWString();
    return reinterpret_cast<INT_PTR>(ShellExecuteW(nullptr,L"open",p.c_str(),a.c_str(),nullptr,SW_HIDE))>32;
}


}
