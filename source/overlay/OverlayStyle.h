#pragma once
#include <QVariantMap>
class OverlayStyle {
public:
    static QString fontFamily(){return "NVIDIA Sans";}
    static int fontWeight(){return 600;}
    static QString line(const QVariantList &items);
    static QVariantMap presentation(const QVariantMap &config,const QString &line);
};
