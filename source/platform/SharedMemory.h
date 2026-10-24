#pragma once
#include <windows.h>
#include <QByteArray>
#include <cstring>
namespace SharedMemory {
struct Mapping {
    HANDLE handle=nullptr; char *ptr=nullptr; size_t capacity=0;
    explicit Mapping(const wchar_t *name,bool write=false) {
        DWORD access=write?(FILE_MAP_READ|FILE_MAP_WRITE):FILE_MAP_READ;
        handle=OpenFileMappingW(access,FALSE,name);
        if(handle) ptr=static_cast<char*>(MapViewOfFile(handle,access,0,0,0));
        if(ptr){MEMORY_BASIC_INFORMATION i{};if(VirtualQuery(ptr,&i,sizeof(i)))capacity=i.RegionSize;}
    }
    ~Mapping(){if(ptr)UnmapViewOfFile(ptr);if(handle)CloseHandle(handle);}
    Mapping(const Mapping&)=delete;
    bool has(size_t p,size_t n)const{return ptr&&p<=capacity&&n<=capacity-p;}
    quint32 u32(size_t p)const {quint32 v=0;if(has(p,4))memcpy(&v,ptr+p,4);return v;}
    QByteArray bytes(size_t p,size_t n)const{return has(p,n)?QByteArray(ptr+p,qsizetype(n)):QByteArray();}
    void put(size_t p,const QByteArray &b){if(has(p,b.size()))memcpy(ptr+p,b.constData(),b.size());}
    bool rtss()const{return has(0,40)&&u32(0)==0x52545353&&u32(4)>=0x20007&&u32(4)<0x30000;}
};
struct MutexGuard {
    HANDLE h=nullptr; bool held=false;
    explicit MutexGuard(const wchar_t *name){h=CreateMutexW(nullptr,FALSE,name);if(h){DWORD r=WaitForSingleObject(h,20);held=r==WAIT_OBJECT_0||r==WAIT_ABANDONED;}}
    ~MutexGuard(){if(held)ReleaseMutex(h);if(h)CloseHandle(h);}
};
static QByteArray terminated(QByteArray b){int i=b.indexOf('\0');if(i>=0)b.truncate(i);return b;}

}
