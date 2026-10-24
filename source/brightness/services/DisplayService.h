#pragma once

#include "brightness/core/DisplayTypes.h"

#include <QList>

class DisplayService
{
public:
    static QList<DisplayInfo> enumerateDisplays();
    static bool apply(const QString &deviceName, const DisplayParameters &parameters);
    static bool reset(const QString &deviceName);
};

