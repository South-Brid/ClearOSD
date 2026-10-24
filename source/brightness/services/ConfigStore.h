#pragma once

#include <QJsonObject>
#include <QString>

class ConfigStore
{
public:
    ConfigStore();

    QJsonObject load(QString *warning = nullptr);
    bool save(const QJsonObject &configuration) const;
    QString path() const;

private:
    QString m_path;
    bool m_writeBlocked = false;
    QString locateLegacyConfig() const;
};
