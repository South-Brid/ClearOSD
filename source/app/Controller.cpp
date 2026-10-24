#include "Controller.h"
#include "model/MetricCatalog.h"
#include "overlay/OverlayStyle.h"
#include "platform/WindowsPlatform.h"
#include <QApplication>
#include <QScreen>
#include <QFileInfo>
#include <QColor>
#include <cmath>
using namespace MetricCatalog;
namespace {
QString trueFpsStatus(RenderedFrameTracker::Status status){
    using Status=RenderedFrameTracker::Status;
    switch(status){
    case Status::Ready:return QStringLiteral("True FPS 已连接");
    case Status::NoTarget:return QStringLiteral("True FPS 等待游戏画面");
    case Status::WaitingForMarkers:return QStringLiteral("True FPS — · 游戏未提供渲染标记");
    case Status::WarmingUp:return QStringLiteral("True FPS 正在采样");
    case Status::Stale:return QStringLiteral("True FPS — · 渲染标记已中断");
    case Status::AccessDenied:return QStringLiteral("True FPS — · 需要管理员权限");
    case Status::EventsLost:return QStringLiteral("True FPS — · 采集丢失事件");
    case Status::StartFailed:case Status::TraceFailed:return QStringLiteral("True FPS — · 采集不可用");
    case Status::Stopped:return QStringLiteral("True FPS 未启用");
    }
    return QStringLiteral("True FPS —");
}
}
Controller::Controller(QObject *parent):QObject(parent){
        connect(this,&Controller::configChanged,this,&Controller::overlayStyleChanged);
        connect(this,&Controller::readingsChanged,this,&Controller::overlayStyleChanged);
        saveTimer.setSingleShot(true);saveTimer.setInterval(350);connect(&saveTimer,&QTimer::timeout,this,&Controller::save);
        moveTimer.setSingleShot(true);moveTimer.setInterval(350);connect(&moveTimer,&QTimer::timeout,this,&Controller::finishDrag);
        connect(&tick,&QTimer::timeout,this,&Controller::sample);tick.start(cfg["refreshIntervalMs"].toInt());
        saveTimer.start();
        QTimer::singleShot(50,this,&Controller::startDependencies);
    }

QVariantMap Controller::config()const{return cfg;}

QVariantMap Controller::overlayStyle()const{return OverlayStyle::presentation(cfg,lineText());}

QVariantList Controller::items()const{
        QVariantList list;for(const auto &key:cfg["metrics"].toStringList()){
            int i=keys.indexOf(key);bool valid=values.contains(key);QString val=valid?QString::number(values[key].toDouble(),'f',key.endsWith("Power")||key=="vram"?1:0):(key=="trueFps"?QStringLiteral("—"):QStringLiteral("--"));
            list<<QVariantMap{{"key",key},{"label",labels[i]},{"value",val},{"unit",units[i]}};
        }return list;
    }

QString Controller::lineText()const{return OverlayStyle::line(items());}

QVariantList Controller::metricOptions()const{
        QVariantList l;for(int i=0;i<keys.size();i++)l<<QVariantMap{{"key",keys[i]},{"title",titles[i]},{"enabled",cfg["metrics"].toStringList().contains(keys[i])},{"available",values.contains(keys[i])}};return l;
    }

QString Controller::status()const{
        if(cfg["metrics"].toStringList().contains("trueFps")&&game.pid)
            return trueFpsStatus(renderedSample.status)+QStringLiteral(" · ")+QFileInfo(game.name).fileName();
        if(values.contains("trueFps"))return trueFpsStatus(renderedSample.status);
        if(!connected)return QStringLiteral("RTSS 未连接 · 桌面监控仍可用");
        if(game.pid)return QStringLiteral("帧率数据已连接 · ")+QFileInfo(game.name).fileName();
        return QStringLiteral("RTSS 已连接 · 等待游戏画面");
    }

QString Controller::sensorNote()const{
        QStringList notes,missing;
        const auto selected=cfg["metrics"].toStringList();
        if(selected.contains("fps"))notes<<QStringLiteral("FPS 来自 RTSS，在驱动插帧时可能包含生成帧。");
        if(selected.contains("trueFps")){
            notes<<QStringLiteral("True FPS 独立统计游戏的渲染提交标记；没有有效数据时显示 —。");
            if(renderedSample.status!=RenderedFrameTracker::Status::Ready)
                notes<<trueFpsStatus(renderedSample.status)+QStringLiteral("。");
        }
        for(const auto &k:selected)if(k!="fps"&&k!="trueFps"&&!values.contains(k))missing<<titles[keys.indexOf(k)];
        notes<< (missing.isEmpty()?QStringLiteral("硬件数据来自 RTSS 自带的 LHM 采集组件。"):missing.join(QStringLiteral("、"))+QStringLiteral("：采集组件未返回有效值，显示 —。"));
        return notes.join(QStringLiteral("\n"));
    }

QStringList Controller::screenNames()const{QStringList l;int i=0;for(auto s:qApp->screens())l<<QString::number(++i)+" · "+s->name();return l;}

bool Controller::desktopVisible()const{return !cfg["hidden"].toBool();}

void Controller::attach(QQuickWindow *w){overlay=w;if(!w)return;connect(w,&QWindow::xChanged,this,[this]{if(dragging)moveTimer.start();});connect(w,&QWindow::yChanged,this,[this]{if(dragging)moveTimer.start();});connect(w,&QWindow::widthChanged,this,[this]{QTimer::singleShot(0,this,&Controller::place);});connect(w,&QWindow::heightChanged,this,[this]{QTimer::singleShot(0,this,&Controller::place);});QTimer::singleShot(0,this,&Controller::place);}

void Controller::change(const QString &key,const QVariant &value){
        if(!cfg.contains(key)||key=="metrics"||key=="x"||key=="y"||key=="osdX"||key=="osdY")return;
        QVariant v=value;if(key=="fontSize")v=qBound(10,value.toInt(),40);
        if(key=="refreshIntervalMs")v=SettingsStore::normalizeRefreshInterval(value);
        if(key=="screen")v=qBound(0,value.toInt(),int(qApp->screens().size()-1));
        if(key=="backgroundOpacity")v=qBound(0.0,value.toDouble(),1.0);
        if(key=="edgeMargin")v=qBound(0,value.toInt(),200);
        if(key=="anchor"&&!SettingsStore::anchors().contains(value.toString()))return;
        if(key=="textColor"||key=="backgroundColor"){QColor c(value.toString());if(!c.isValid())return;v=c.name();}
        if(cfg[key]==v)return;cfg[key]=v;
        // Free dragging starts from wherever the overlay currently is.
        if(key=="anchor"&&v=="custom"&&overlay){cfg["x"]=overlay->x();cfg["y"]=overlay->y();}
        if(key=="refreshIntervalMs")tick.setInterval(v.toInt());
        emit configChanged();emit statusChanged();saveTimer.start();
        if(key=="anchor"||key=="fontSize"||key=="screen"||key=="edgeMargin")QTimer::singleShot(0,this,&Controller::place);
    }

void Controller::toggleMetric(const QString &key,bool enabled){
        if(!keys.contains(key))return;auto l=cfg["metrics"].toStringList();
        if(enabled&&!l.contains(key))l<<key;else if(!enabled&&l.size()>1)l.removeAll(key);
        cfg["metrics"]=l;emit configChanged();emit readingsChanged();saveTimer.start();QTimer::singleShot(0,this,&Controller::place);
        if(key=="trueFps"){
            if(enabled)renderedTracker.start();else renderedTracker.stop();
            sample();
        }
    }

void Controller::openSettings(){emit showSettings();}

void Controller::quit(){qApp->quit();}

void Controller::toggleHidden(){change("hidden",!cfg["hidden"].toBool());}

void Controller::beginDrag(){if(!cfg["locked"].toBool())dragging=true;}

void Controller::finishDrag(){
        if(!dragging||!overlay)return;dragging=false;cfg["anchor"]="custom";
        cfg["x"]=overlay->x();cfg["y"]=overlay->y();auto s=overlay->screen();auto g=s->geometry();auto d=s->devicePixelRatio();
        cfg["osdX"]=qRound((overlay->x()-g.x())*d);cfg["osdY"]=qRound((overlay->y()-g.y())*d);cfg["screen"]=qApp->screens().indexOf(s);
        emit configChanged();saveTimer.start();
    }

void Controller::place(){
        if(!overlay||dragging)return;auto screens=qApp->screens();auto s=screens[qBound(0,cfg["screen"].toInt(),int(screens.size()-1))];auto g=s->availableGeometry();auto a=cfg["anchor"].toString();
        // Anchor names are "<row><Column>": top/center/bottom + Left/Center/Right.
        const int w=overlay->width(),h=overlay->height(),m=cfg["edgeMargin"].toInt();
        int x=a.endsWith("Left")?g.left()+m:a.endsWith("Right")?g.right()+1-w-m:g.left()+(g.width()-w)/2;
        int y=a.startsWith("top")?g.top()+m:a.startsWith("bottom")?g.bottom()+1-h-m:g.top()+(g.height()-h)/2;
        if(a=="custom"){x=cfg["x"].toInt();y=cfg["y"].toInt();}
        overlay->setPosition(qMax(g.left(),qMin(x,g.right()-w+1)),qMax(g.top(),qMin(y,g.bottom()-h+1)));
    }

void Controller::startDependencies(){
        rtss.start();
        sensors.start();
        if(cfg["metrics"].toStringList().contains("trueFps"))renderedTracker.start();
        sample();
    }

void Controller::sample(){
        if(closing)return;
        values=sensors.sample();
        connected=RtssClient::isConnected();
        game=RtssClient::foregroundGame();if(game.pid&&game.fps>=0)values["fps"]=game.fps;
        renderedTracker.setTargetProcess(game.pid?game.pid:WindowsPlatform::foregroundProcessId());
        renderedSample=renderedTracker.sample();
        if(cfg["metrics"].toStringList().contains("trueFps")&&
           renderedSample.status==RenderedFrameTracker::Status::Ready&&
           std::isfinite(renderedSample.fps)&&renderedSample.fps>=0)
            values["trueFps"]=renderedSample.fps;
        emit readingsChanged();emit statusChanged();
    }

void Controller::save(){SettingsStore::save(cfg);}

void Controller::close(){closing=true;tick.stop();saveTimer.stop();save();renderedTracker.stop();sensors.stop();}

QVariantMap Controller::diagnostics()const{
        QVariantMap geometry;
        if(overlay)geometry={{"x",overlay->x()},{"y",overlay->y()},{"width",overlay->width()},{"height",overlay->height()}};
        QVariantMap frameSource{{"source","NVIDIA PCL/Reflex RenderSubmitStart"},
            {"status",trueFpsStatus(renderedSample.status)},
            {"errorCode",renderedSample.errorCode},{"pid",renderedSample.processId},
            {"renderedFrames",QVariant::fromValue<qulonglong>(renderedSample.renderedFrames)},
            {"providerEvents",QVariant::fromValue<qulonglong>(renderedSample.providerEvents)},
            {"decodeFailures",QVariant::fromValue<qulonglong>(renderedSample.decodeFailures)},
            {"recentRenderedFrames",renderedSample.recentRenderedFrames},
            {"lastMarkerAgeMs",QVariant::fromValue<qulonglong>(renderedSample.lastMarkerAgeMs)}};
        return {{"status",status()},{"values",values},{"config",cfg},{"overlay",geometry},{"trueFpsSource",frameSource}};
    }
