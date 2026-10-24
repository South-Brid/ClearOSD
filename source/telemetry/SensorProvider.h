#pragma once
#include <QVariantMap>
#include <QElapsedTimer>
class SensorProvider {
public:
    void start();
    void stop();
    QVariantMap sample();
    static QVariantMap parse(const QByteArray &xml);
private:
    static QVariantMap read();
    bool ownsProcess=false;
    QElapsedTimer staleClock;
    QByteArray previousSensorHash;
};
