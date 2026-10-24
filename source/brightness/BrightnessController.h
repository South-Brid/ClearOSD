#pragma once

#include "brightness/core/DisplayTypes.h"
#include "brightness/services/ConfigStore.h"
#include "brightness/services/HotkeyManager.h"

#include <QHash>
#include <QSet>
#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

class BrightnessController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList monitors READ monitors NOTIFY monitorsChanged)
    Q_PROPERTY(int currentMonitorIndex READ currentMonitorIndex WRITE setCurrentMonitorIndex
                   NOTIFY currentMonitorIndexChanged)
    Q_PROPERTY(double gamma READ gamma WRITE setGamma NOTIFY parametersChanged)
    Q_PROPERTY(double brightness READ brightness WRITE setBrightness NOTIFY parametersChanged)
    Q_PROPERTY(double contrast READ contrast WRITE setContrast NOTIFY parametersChanged)
    Q_PROPERTY(QStringList profiles READ profiles NOTIFY profilesChanged)
    Q_PROPERTY(QString selectedProfile READ selectedProfile NOTIFY selectedProfileChanged)
    Q_PROPERTY(QString selectedHotkey READ selectedHotkey NOTIFY selectedHotkeyChanged)
    Q_PROPERTY(QVariantMap profileHotkeys READ profileHotkeys NOTIFY profileHotkeysChanged)
    Q_PROPERTY(bool startupEnabled READ startupEnabled WRITE setStartupEnabled
                   NOTIFY startupEnabledChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString configPath READ configPath CONSTANT)

public:
    explicit BrightnessController(QObject *parent = nullptr);

    QVariantList monitors() const;
    int currentMonitorIndex() const;
    double gamma() const;
    double brightness() const;
    double contrast() const;
    QStringList profiles() const;
    QString selectedProfile() const;
    QString selectedHotkey() const;
    QVariantMap profileHotkeys() const;
    bool startupEnabled() const;
    QString statusText() const;
    QString configPath() const;

    void setCurrentMonitorIndex(int index);
    void setGamma(double value);
    void setBrightness(double value);
    void setContrast(double value);
    void setStartupEnabled(bool enabled);

    Q_INVOKABLE void resetParameter(const QString &parameter);
    Q_INVOKABLE void resetCurrent();
    Q_INVOKABLE void resetAll();
    Q_INVOKABLE void syncAll();
    Q_INVOKABLE void selectProfile(const QString &name);
    Q_INVOKABLE bool saveProfile(const QString &name, const QString &hotkey);
    Q_INVOKABLE bool deleteProfile(const QString &name);
    Q_INVOKABLE void unbindHotkey(const QString &name);

signals:
    void monitorsChanged();
    void currentMonitorIndexChanged();
    void parametersChanged();
    void profilesChanged();
    void selectedProfileChanged();
    void selectedHotkeyChanged();
    void profileHotkeysChanged();
    void startupEnabledChanged();
    void statusTextChanged();

private:
    DisplayParameters currentParameters() const;
    void setParameter(const QString &name, double value);
    bool applyCurrent();
    void applyProfile(const QString &name, const QString &statusTemplate);
    void applyProfileFromHotkey(const QString &name);
    void ensureConfigurationDefaults();
    void loadDisplayParameters();
    void refreshDisplays();
    void refreshProfiles();
    void restoreHotkeys();
    void restoreDefaultsOnExit();
    bool persist();
    void setStatus(const QString &text);
    QJsonObject profileObject(const QString &name) const;

    ConfigStore m_store;
    HotkeyManager m_hotkeys;
    QTimer m_displayRefresh;
    QJsonObject m_configuration;
    QList<DisplayInfo> m_displays;
    QHash<QString, DisplayParameters> m_parameters;
    QSet<QString> m_modifiedDisplays;
    QStringList m_profiles;
    QString m_selectedProfile = QStringLiteral("default");
    QString m_selectedHotkey;
    QString m_statusText = QStringLiteral("就绪");
    int m_currentMonitorIndex = 0;
    bool m_startupEnabled = false;
};
