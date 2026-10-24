#pragma once
#include <QByteArray>
#include <QString>
struct Game { quint32 pid=0; double fps=-1; QString name; };
class RtssClient {
public:
    void start();
    static Game foregroundGame();
    static bool isConnected();
    static bool clearLegacyOsd();
};
