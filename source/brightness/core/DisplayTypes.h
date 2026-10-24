#pragma once

#include <QString>

struct DisplayInfo
{
    QString deviceName;
    QString label;
    int width = 0;
    int height = 0;
    bool primary = false;
};

struct DisplayParameters
{
    double gamma = 1.0;
    double brightness = 0.0;
    double contrast = 1.0;
};

