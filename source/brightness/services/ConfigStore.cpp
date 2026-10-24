#include "brightness/services/ConfigStore.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>

ConfigStore::ConfigStore()
{
    // Portable configuration: load and save beside this executable.
    m_path = QDir(QCoreApplication::applicationDirPath())
                 .filePath(QStringLiteral("gamma_config.json"));

    if (!QFileInfo::exists(m_path)) {
        const QString legacy = locateLegacyConfig();
        if (!legacy.isEmpty()) {
            QFile::copy(legacy, m_path);
        }
    }
}

QJsonObject ConfigStore::load(QString *warning)
{
    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(data, &error);
    if (error.error == QJsonParseError::NoError && document.isObject()) {
        return document.object();
    }

    // Keep the unreadable file: the next save would otherwise replace the
    // user's profiles with an empty configuration.
    const QString backup = QStringLiteral("%1.broken-%2").arg(
        m_path, QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")));
    if (QFile::copy(m_path, backup)) {
        if (warning)
            *warning = QStringLiteral("配置文件格式错误，已备份为 %1").arg(QFileInfo(backup).fileName());
    } else {
        m_writeBlocked = true;
        if (warning)
            *warning = QStringLiteral("配置文件格式错误且无法备份，本次运行不会写入配置");
    }
    return {};
}

bool ConfigStore::save(const QJsonObject &configuration) const
{
    if (m_writeBlocked) {
        return false;
    }
    QSaveFile file(m_path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(QJsonDocument(configuration).toJson(QJsonDocument::Indented));
    return file.commit();
}

QString ConfigStore::path() const
{
    return m_path;
}

QString ConfigStore::locateLegacyConfig() const
{
    // Only locations tied to this application. The working directory is
    // arbitrary when launched at logon and must not be searched.
    const QString applicationDirectory = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation))
            .filePath(QStringLiteral("gamma_config.json")),
        QDir(applicationDirectory).filePath(QStringLiteral("../gamma_config.json")),
        QDir(applicationDirectory).filePath(QStringLiteral("../../python/gamma_config.json"))
    };

    for (const QString &candidate : candidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isFile()) {
            return info.absoluteFilePath();
        }
    }
    return {};
}
