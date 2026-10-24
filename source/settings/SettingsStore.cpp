#include "SettingsStore.h"
#include "platform/WindowsPlatform.h"
#include "model/MetricCatalog.h"
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QColor>
#include <QDateTime>
using namespace WindowsPlatform;
using namespace MetricCatalog;
const QStringList &SettingsStore::anchors(){
    static const QStringList values{"topLeft","topCenter","topRight","centerLeft","center","centerRight","bottomLeft","bottomCenter","bottomRight","custom"};
    return values;
}
QVariantMap SettingsStore::defaults(){return {{"refreshIntervalMs",700},{"fontSize",18},{"textColor","#ffffff"},{"backgroundColor","#111820"},{"backgroundOpacity",0.42},{"anchor","topRight"},{"edgeMargin",14},{"locked",false},{"hidden",false},{"screen",0},{"x",100},{"y",20},{"osdX",100},{"osdY",20},{"frameRateSchemaVersion",2},{"metrics",QStringList{"fps","trueFps","gpu","cpu","ram","cpuPower","gpuPower","cpuTemp","gpuTemp"}}};}

int SettingsStore::normalizeRefreshInterval(const QVariant &value) {
    bool valid=false;
    const int interval=value.toInt(&valid);
    return valid?qBound(100,interval,10000):700;
}

QVariantMap SettingsStore::load(){
    auto cfg=defaults();
        QFile f(dataDir()+"/settings.json");if(f.open(QIODevice::ReadOnly)){
            QJsonParseError error;
            const auto document=QJsonDocument::fromJson(f.readAll(),&error);
            f.close();
            // The first autosave would otherwise replace an unreadable file with defaults.
            if(error.error!=QJsonParseError::NoError||!document.isObject())
                QFile::copy(f.fileName(),f.fileName()+".broken-"+QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss"));
            auto saved=document.object().toVariantMap();
            for(auto i=cfg.begin();i!=cfg.end();++i)if(saved.contains(i.key()))i.value()=saved[i.key()];
            if(saved.value("frameRateSchemaVersion",0).toInt()<2){
                auto selected=cfg["metrics"].toStringList();
                const int fpsIndex=selected.indexOf("fps");
                if(fpsIndex>=0&&!selected.contains("trueFps"))selected.insert(fpsIndex+1,"trueFps");
                cfg["metrics"]=selected;
            }
        }
        cfg["refreshIntervalMs"]=normalizeRefreshInterval(cfg["refreshIntervalMs"]);
        cfg["fontSize"]=qBound(10,cfg["fontSize"].toInt(),40);
        cfg["backgroundOpacity"]=qBound(0.0,cfg["backgroundOpacity"].toDouble(),1.0);
        cfg["edgeMargin"]=qBound(0,cfg["edgeMargin"].toInt(),200);
        if(!anchors().contains(cfg["anchor"].toString()))cfg["anchor"]=defaults()["anchor"];
        for(auto k:{"textColor","backgroundColor"})if(!QColor(cfg[k].toString()).isValid())cfg[k]=defaults()[k];
        QStringList selected;for(const auto &k:cfg["metrics"].toStringList())if(keys.contains(k)&&!selected.contains(k))selected<<k;
        cfg["metrics"]=selected.isEmpty()?QStringList{"fps","trueFps"}:selected;
        cfg["frameRateSchemaVersion"]=2;
    return cfg;
}
void SettingsStore::save(const QVariantMap &config) {
    QSaveFile file(dataDir()+"/settings.json");
    if(file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(QJsonObject::fromVariantMap(config)).toJson());
        file.commit();
    }
}
