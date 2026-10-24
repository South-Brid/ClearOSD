#pragma once
#include <QObject>
#include <QVariantMap>
#include <QTimer>
#include <QQuickWindow>
#include "settings/SettingsStore.h"
#include "rtss/RtssClient.h"
#include "telemetry/SensorProvider.h"
#include "telemetry/RenderedFrameTracker.h"
class Controller:public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap config READ config NOTIFY configChanged)
    Q_PROPERTY(QVariantMap overlayStyle READ overlayStyle NOTIFY overlayStyleChanged)
    Q_PROPERTY(QVariantList items READ items NOTIFY readingsChanged)
    Q_PROPERTY(QString lineText READ lineText NOTIFY readingsChanged)
    Q_PROPERTY(QVariantList metricOptions READ metricOptions NOTIFY readingsChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString sensorNote READ sensorNote NOTIFY readingsChanged)
    Q_PROPERTY(QStringList screenNames READ screenNames CONSTANT)
    Q_PROPERTY(bool desktopVisible READ desktopVisible NOTIFY statusChanged)
    QVariantMap cfg=SettingsStore::load(),values;
    Game game;RtssClient rtss;SensorProvider sensors;RenderedFrameTracker renderedTracker;
    RenderedFrameTracker::Sample renderedSample;
    QTimer tick,saveTimer,moveTimer;
    QQuickWindow *overlay=nullptr;bool connected=false,dragging=false,closing=false;
public:
    explicit Controller(QObject *parent=nullptr);
    QVariantMap config()const;
    QVariantMap overlayStyle()const;
    QVariantList items()const;
    QString lineText()const;
    QVariantList metricOptions()const;
    QString status()const;
    QString sensorNote()const;
    QStringList screenNames()const;
    bool desktopVisible()const;
    void attach(QQuickWindow *w);
public slots:
    void change(const QString &key,const QVariant &value);
    void toggleMetric(const QString &key,bool enabled);
    void openSettings();
    void quit();
    void toggleHidden();
    void beginDrag();
    void finishDrag();
    void place();
    void startDependencies();
    void sample();
    void save();
    void close();
    QVariantMap diagnostics()const;
signals:
    void overlayStyleChanged();
    void configChanged();void readingsChanged();void statusChanged();void showSettings();
};
