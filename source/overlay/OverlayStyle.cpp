#include "OverlayStyle.h"
#include <QStringList>
#include <QMap>
QString OverlayStyle::line(const QVariantList &items) {
    QStringList pieces;
    for(const auto &value:items) {
        const auto item=value.toMap();
        pieces<<item["label"].toString()+" "+item["value"].toString()+item["unit"].toString();
    }
    return pieces.join("  |  ");
}
QVariantMap OverlayStyle::presentation(const QVariantMap &config,const QString &line) {
    return {{"fontFamily",fontFamily()},{"fontWeight",fontWeight()},
        {"fontSize",config["fontSize"]},{"text",line},
        {"textColor",config["textColor"]},{"backgroundColor",config["backgroundColor"]},
        {"backgroundOpacity",config["backgroundOpacity"]}};
}
