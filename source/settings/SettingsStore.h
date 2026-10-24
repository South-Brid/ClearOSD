#pragma once
#include <QStringList>
#include <QVariantMap>
class SettingsStore {
public:
    // 3x3 grid, row-major, followed by free dragging.
    static const QStringList &anchors();
    static int normalizeRefreshInterval(const QVariant &value);
    static QVariantMap defaults();
    static QVariantMap load();
    static void save(const QVariantMap &config);
};
