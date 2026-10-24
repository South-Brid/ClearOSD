#include "RtssClient.h"
#include "platform/WindowsPlatform.h"
#include "platform/SharedMemory.h"
#include <QLibrary>
#include <cmath>
using namespace WindowsPlatform;
using namespace SharedMemory;
Game RtssClient::foregroundGame() {
    const DWORD pid=foregroundProcessId();
    if(!pid)return {};
    Mapping m(L"RTSSSharedMemoryV2");if(!m.rtss())return {};
    quint32 size=m.u32(8),off=m.u32(12),count=m.u32(16);
    if(size<284||count>4096||!m.has(off,size_t(size)*count))return {};
    for(quint32 i=0;i<count;i++){
        size_t p=off+size_t(size)*i;if(m.u32(p)!=pid)continue;
        quint32 t0=m.u32(p+268),t1=m.u32(p+272),frames=m.u32(p+276),dt=t1-t0;
        Game game{pid,-1,QString::fromLocal8Bit(terminated(m.bytes(p+4,260)))};
        if(!t0||!dt||dt>10000||DWORD(GetTickCount()-t1)>2500)return game;
        double fps=1000.0*frames/dt;if(!std::isfinite(fps)||fps>5000)return game;
        game.fps=fps;return game;
    }
    return {};
}
bool RtssClient::clearLegacyOsd(){
    Mapping m(L"RTSSSharedMemoryV2",true);if(!m.rtss())return false;
    quint32 size=m.u32(20),off=m.u32(24),count=m.u32(28);
    if(size<4608||count>4096||!m.has(off,size_t(size)*count))return false;
    volatile LONG *lock=m.u32(4)>=0x2000e?reinterpret_cast<volatile LONG*>(m.ptr+36):nullptr;
    if(lock&&InterlockedCompareExchange(lock,1,0)!=0)return false;
    struct Unlock{volatile LONG *p;~Unlock(){if(p)InterlockedExchange(p,0);}}unlock{lock};
    for(quint32 i=1;i<count;i++){
        size_t p=off+size_t(size)*i;
        const auto owner=terminated(m.bytes(p+256,256));
        if(owner!="RTSSOverlayEditor"&&owner!="ClearOSD.RenderProbe"&&owner!="ClearOSD.Native.v1")continue;
        auto text=terminated(m.bytes(p+(size>=299520?266752:512),size>=299520?32768:4096));
        // Only retire the one-line template created in this session.
        if(owner=="RTSSOverlayEditor"&&(!text.contains("<FNT=NVIDIA Sans,-9,600,2>")||!text.contains("FPS <FR>   |   GPU")))continue;
        m.put(p,QByteArray(1,'\0'));m.put(p+512,QByteArray(1,'\0'));
        if(size>=299520)m.put(p+266752,QByteArray(1,'\0'));
        m.put(p+256,QByteArray(1,'\0'));
    }
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(m.ptr+32));return true;
}

void RtssClient::start() {
    if(!processId("RTSS.exe"))launchHidden(rtssDir()+"/RTSS.exe");
}

bool RtssClient::isConnected(){Mapping memory(L"RTSSSharedMemoryV2");return memory.rtss();}
