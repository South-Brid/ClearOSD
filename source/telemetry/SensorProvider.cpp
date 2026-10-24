#include "SensorProvider.h"
#include "platform/WindowsPlatform.h"
#include "platform/SharedMemory.h"
#include <QXmlStreamReader>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <cmath>
using namespace WindowsPlatform;
using namespace SharedMemory;
QVariantMap SensorProvider::parse(const QByteArray &xml) {
    QVariantMap values;
    // The bundled provider exports sibling <hardware> elements, not one root.
    QXmlStreamReader r(QByteArray("<root>")+xml+"</root>");
    while(!r.atEnd()){
        r.readNext();if(!r.isStartElement()||r.name()!=u"sensor")continue;
        QString id,name,type,raw;
        while(r.readNextStartElement()){
            QString key=r.name().toString();QString text=r.readElementText();
            if(key=="id")id=text.toLower();else if(key=="name")name=text.toLower();
            else if(key=="type")type=text.toLower();else if(key=="value")raw=text;
        }
        bool ok=false;double v=raw.replace(',','.').toDouble(&ok);
        if(!ok||!std::isfinite(v)||v<0||v>100000)continue;
        if(type=="load"&&name=="cpu total"&&v<=100)values["cpu"]=v;
        if(id=="/ram/load/0"&&v<=100)values["ram"]=v;
        if(type=="power"&&name=="cpu package"&&v>0)values["cpuPower"]=v;
        if(type=="temperature"&&name=="cpu package"&&v<150)values["cpuTemp"]=v;
        // A zero-only CPU Memory channel is not evidence of real DIMM power.
        if(type=="power"&&(name=="cpu memory"||name=="dram"||name=="dram power")&&v>0)values["ramPower"]=v;
        if(id.startsWith("/gpu-nvidia/0/")){
            if(type=="load"&&name=="gpu core"&&v<=100)values["gpu"]=v;
            if(type=="power"&&(name=="gpu package"||name=="gpu power"))values["gpuPower"]=v;
            if(type=="temperature"&&name=="gpu core"&&v<150)values["gpuTemp"]=v;
            if(type=="smalldata"&&name=="gpu memory used")values["vram"]=v/1024;
        }
    }
    if(r.hasError())return {};return values;
}
QVariantMap SensorProvider::read() {
    if(!processId("LHMDataProvider.exe"))return {};
    Mapping m(L"LHMDPSharedMemory");if(!m.has(0,1048576))return {};
    QByteArray xml;
    {MutexGuard g(L"Global\\Access_LHMDPSharedMemory");if(!g.held)return {};xml=terminated(m.bytes(0,1048576));}
    return parse(xml);
}

void SensorProvider::start() {
    const auto provider=rtssDir()+"/Plugins/Client/LHMDataProvider/LHMDataProvider.exe";
    if(!processId("LHMDataProvider.exe")&&QFileInfo::exists(provider))
        ownsProcess=launchHidden(provider,"-i");
}
void SensorProvider::stop() {
    if(ownsProcess)launchHidden(rtssDir()+"/Plugins/Client/LHMDataProvider/LHMDataProvider.exe","-u");
    ownsProcess=false;
}
QVariantMap SensorProvider::sample() {
    auto values=read();
    const auto digest=QJsonDocument(QJsonObject::fromVariantMap(values)).toJson(QJsonDocument::Compact);
    if(digest!=previousSensorHash){previousSensorHash=digest;staleClock.restart();}
    else if(staleClock.isValid()&&staleClock.elapsed()>15000)values.clear();
    return values;
}
