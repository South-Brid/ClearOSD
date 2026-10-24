#include "brightness/BrightnessController.h"

#include "brightness/services/DisplayService.h"
#include "brightness/services/StartupManager.h"

#include <QDir>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QScreen>
#include <QtMath>

#include <algorithm>
#include <cmath>

namespace {

constexpr auto builtInProfile = "default";

double bounded(const QString &name, double value)
{
    if (name == QStringLiteral("gamma")) {
        return std::clamp(value, 0.2, 3.0);
    }
    if (name == QStringLiteral("brightness")) {
        return std::clamp(value, -128.0, 128.0);
    }
    return std::clamp(value, 0.1, 3.0);
}

DisplayParameters parametersFromJson(const QJsonObject &object)
{
    DisplayParameters parameters;
    parameters.gamma = bounded(QStringLiteral("gamma"),
                               object.value(QStringLiteral("gamma")).toDouble(1.0));
    parameters.brightness = bounded(
        QStringLiteral("brightness"),
        object.value(QStringLiteral("brightness")).toDouble(0.0));
    parameters.contrast = bounded(
        QStringLiteral("contrast"),
        object.value(QStringLiteral("contrast")).toDouble(1.0));
    return parameters;
}

QJsonObject parametersToJson(const DisplayParameters &parameters)
{
    return {
        {QStringLiteral("gamma"), parameters.gamma},
        {QStringLiteral("brightness"), parameters.brightness},
        {QStringLiteral("contrast"), parameters.contrast}
    };
}

} // namespace

BrightnessController::BrightnessController(QObject *parent)
    : QObject(parent)
    , m_displays(DisplayService::enumerateDisplays())
    , m_startupEnabled(StartupManager::isEnabled())
{
    QString configurationWarning;
    m_configuration = m_store.load(&configurationWarning);
    ensureConfigurationDefaults();

    loadDisplayParameters();
    refreshProfiles();

    m_selectedHotkey = profileObject(m_selectedProfile)
                           .value(QStringLiteral("_hotkey")).toString();
    connect(&m_hotkeys, &HotkeyManager::activated,
            this, &BrightnessController::applyProfileFromHotkey);
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
            this, &BrightnessController::restoreDefaultsOnExit);

    // Screen notifications arrive in bursts while Windows reconfigures.
    m_displayRefresh.setSingleShot(true);
    m_displayRefresh.setInterval(600);
    connect(&m_displayRefresh, &QTimer::timeout, this, &BrightnessController::refreshDisplays);
    const auto scheduleRefresh = [this] { m_displayRefresh.start(); };
    connect(qGuiApp, &QGuiApplication::screenAdded, this, scheduleRefresh);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, scheduleRefresh);
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, scheduleRefresh);

    if (!configurationWarning.isEmpty()) {
        setStatus(configurationWarning);
    } else if (m_displays.isEmpty()) {
        setStatus(QStringLiteral("未检测到可调节的显示器"));
    } else {
        setStatus(QStringLiteral("当前：%1").arg(m_displays.constFirst().label));
    }
    // Registered after the startup status so failures stay visible.
    restoreHotkeys();
}

QVariantList BrightnessController::monitors() const
{
    QVariantList result;
    for (qsizetype index = 0; index < m_displays.size(); ++index) {
        const DisplayInfo &display = m_displays[index];
        result.append(QVariantMap{
            {QStringLiteral("index"), index},
            {QStringLiteral("device"), display.deviceName},
            {QStringLiteral("display"), display.label},
            {QStringLiteral("width"), display.width},
            {QStringLiteral("height"), display.height},
            {QStringLiteral("primary"), display.primary}
        });
    }
    return result;
}

int BrightnessController::currentMonitorIndex() const { return m_currentMonitorIndex; }
double BrightnessController::gamma() const { return currentParameters().gamma; }
double BrightnessController::brightness() const { return currentParameters().brightness; }
double BrightnessController::contrast() const { return currentParameters().contrast; }
QStringList BrightnessController::profiles() const { return m_profiles; }
QString BrightnessController::selectedProfile() const { return m_selectedProfile; }
QString BrightnessController::selectedHotkey() const { return m_selectedHotkey; }

QVariantMap BrightnessController::profileHotkeys() const
{
    QVariantMap result;
    const QJsonObject profilesObject = m_configuration.value(QStringLiteral("profiles")).toObject();
    for (auto iterator = profilesObject.constBegin(); iterator != profilesObject.constEnd(); ++iterator) {
        const QString hotkey = iterator.value().toObject().value(QStringLiteral("_hotkey")).toString();
        if (!hotkey.isEmpty()) {
            result.insert(iterator.key(), hotkey);
        }
    }
    return result;
}
bool BrightnessController::startupEnabled() const { return m_startupEnabled; }
QString BrightnessController::statusText() const { return m_statusText; }
QString BrightnessController::configPath() const { return m_store.path(); }

void BrightnessController::setCurrentMonitorIndex(int index)
{
    if (index < 0 || index >= m_displays.size() || index == m_currentMonitorIndex) {
        return;
    }
    m_currentMonitorIndex = index;
    emit currentMonitorIndexChanged();
    emit parametersChanged();
    setStatus(QStringLiteral("当前：%1").arg(m_displays[index].label));
}

void BrightnessController::setGamma(double value) { setParameter(QStringLiteral("gamma"), value); }
void BrightnessController::setBrightness(double value) { setParameter(QStringLiteral("brightness"), value); }
void BrightnessController::setContrast(double value) { setParameter(QStringLiteral("contrast"), value); }

void BrightnessController::setStartupEnabled(bool enabled)
{
    if (m_startupEnabled == enabled) {
        return;
    }
    if (!StartupManager::setEnabled(enabled)) {
        setStatus(QStringLiteral("开机启动设置失败，请检查注册表访问权限"));
        emit startupEnabledChanged();
        return;
    }
    m_startupEnabled = enabled;
    emit startupEnabledChanged();
    setStatus(enabled ? QStringLiteral("已开启开机启动")
                      : QStringLiteral("已关闭开机启动"));
}

void BrightnessController::resetParameter(const QString &parameter)
{
    if (parameter == QStringLiteral("gamma")) {
        setGamma(1.0);
    } else if (parameter == QStringLiteral("brightness")) {
        setBrightness(0.0);
    } else if (parameter == QStringLiteral("contrast")) {
        setContrast(1.0);
    }
}

void BrightnessController::resetCurrent()
{
    if (m_displays.isEmpty()) {
        return;
    }
    const DisplayInfo &display = m_displays[m_currentMonitorIndex];
    m_parameters.insert(display.deviceName, DisplayParameters{});
    DisplayService::reset(display.deviceName);
    emit parametersChanged();
    setStatus(QStringLiteral("%1 已恢复默认").arg(display.label));
}

void BrightnessController::resetAll()
{
    for (const DisplayInfo &display : m_displays) {
        m_parameters.insert(display.deviceName, DisplayParameters{});
        DisplayService::reset(display.deviceName);
    }
    emit parametersChanged();
    setStatus(QStringLiteral("全部显示器已恢复默认"));
}

void BrightnessController::syncAll()
{
    if (m_displays.isEmpty()) {
        return;
    }
    const DisplayParameters parameters = currentParameters();
    for (const DisplayInfo &display : m_displays) {
        m_parameters.insert(display.deviceName, parameters);
        if (DisplayService::apply(display.deviceName, parameters)) {
            m_modifiedDisplays.insert(display.deviceName);
        }
    }
    emit parametersChanged();
    setStatus(QStringLiteral("当前参数已同步到全部显示器"));
}

void BrightnessController::selectProfile(const QString &name)
{
    applyProfile(name.trimmed(), QStringLiteral("已载入方案“%1”"));
}

bool BrightnessController::saveProfile(const QString &name, const QString &hotkey)
{
    const QString normalizedName = name.trimmed();
    const QString normalizedHotkey = hotkey.trimmed();
    if (m_displays.isEmpty()) {
        setStatus(QStringLiteral("未检测到可调节的显示器"));
        return false;
    }
    if (normalizedName.isEmpty()) {
        setStatus(QStringLiteral("请输入方案名称"));
        return false;
    }

    const QJsonObject previous = profileObject(normalizedName);
    const QString previousHotkey = previous.value(QStringLiteral("_hotkey")).toString();

    if (!normalizedHotkey.isEmpty()) {
        QString error;
        if (!m_hotkeys.registerHotkey(normalizedName, normalizedHotkey, &error)) {
            if (!previousHotkey.isEmpty()) {
                m_hotkeys.registerHotkey(normalizedName, previousHotkey, nullptr);
            }
            setStatus(QStringLiteral("保存失败：%1").arg(error));
            return false;
        }
    } else {
        m_hotkeys.unregisterOwner(normalizedName);
    }

    QJsonObject profile = parametersToJson(currentParameters());
    if (!normalizedHotkey.isEmpty()) {
        profile.insert(QStringLiteral("_hotkey"), normalizedHotkey);
    }
    QJsonObject profilesObject = m_configuration.value(QStringLiteral("profiles")).toObject();
    profilesObject.insert(normalizedName, profile);
    m_configuration.insert(QStringLiteral("profiles"), profilesObject);

    m_selectedProfile = normalizedName;
    m_selectedHotkey = normalizedHotkey;
    refreshProfiles();
    emit selectedProfileChanged();
    emit selectedHotkeyChanged();
    if (!persist()) {
        return false;
    }
    setStatus(normalizedHotkey.isEmpty()
                  ? QStringLiteral("方案“%1”已保存").arg(normalizedName)
                  : QStringLiteral("方案“%1”已保存 · %2 已绑定")
                        .arg(normalizedName, normalizedHotkey));
    return true;
}

bool BrightnessController::deleteProfile(const QString &name)
{
    const QString normalized = name.trimmed();
    if (normalized == QString::fromLatin1(builtInProfile)) {
        setStatus(QStringLiteral("内置 default 方案不可删除"));
        return false;
    }

    QJsonObject profilesObject = m_configuration.value(QStringLiteral("profiles")).toObject();
    if (!profilesObject.contains(normalized)) {
        setStatus(QStringLiteral("方案“%1”不存在").arg(normalized));
        return false;
    }
    m_hotkeys.unregisterOwner(normalized);
    profilesObject.remove(normalized);
    m_configuration.insert(QStringLiteral("profiles"), profilesObject);
    m_selectedProfile = QString::fromLatin1(builtInProfile);
    m_selectedHotkey = profileObject(m_selectedProfile)
                           .value(QStringLiteral("_hotkey")).toString();
    refreshProfiles();
    emit selectedProfileChanged();
    emit selectedHotkeyChanged();
    if (!persist()) {
        return false;
    }
    setStatus(QStringLiteral("方案“%1”已删除").arg(normalized));
    return true;
}

void BrightnessController::unbindHotkey(const QString &name)
{
    const QString normalized = name.trimmed();
    QJsonObject profilesObject = m_configuration.value(QStringLiteral("profiles")).toObject();
    QJsonObject profile = profilesObject.value(normalized).toObject();
    if (profile.isEmpty()) {
        setStatus(QStringLiteral("方案“%1”不存在").arg(normalized));
        return;
    }
    if (!profile.contains(QStringLiteral("_hotkey"))) {
        setStatus(QStringLiteral("方案“%1”没有绑定热键").arg(normalized));
        return;
    }
    m_hotkeys.unregisterOwner(normalized);
    profile.remove(QStringLiteral("_hotkey"));
    profilesObject.insert(normalized, profile);
    m_configuration.insert(QStringLiteral("profiles"), profilesObject);
    emit profileHotkeysChanged();
    if (normalized == m_selectedProfile) {
        m_selectedHotkey.clear();
        emit selectedHotkeyChanged();
    }
    if (persist()) {
        setStatus(QStringLiteral("方案“%1”的热键已解绑").arg(normalized));
    }
}

DisplayParameters BrightnessController::currentParameters() const
{
    if (m_displays.isEmpty()) {
        return {};
    }
    return m_parameters.value(m_displays[m_currentMonitorIndex].deviceName,
                              DisplayParameters{});
}

void BrightnessController::setParameter(const QString &name, double value)
{
    if (m_displays.isEmpty() || !std::isfinite(value)) {
        return;
    }
    DisplayParameters parameters = currentParameters();
    const double clamped = bounded(name, value);
    if (name == QStringLiteral("gamma")) {
        if (qFuzzyCompare(parameters.gamma, clamped)) return;
        parameters.gamma = clamped;
    } else if (name == QStringLiteral("brightness")) {
        if (qFuzzyCompare(parameters.brightness + 1.0, clamped + 1.0)) return;
        parameters.brightness = clamped;
    } else if (name == QStringLiteral("contrast")) {
        if (qFuzzyCompare(parameters.contrast, clamped)) return;
        parameters.contrast = clamped;
    } else {
        return;
    }

    m_parameters.insert(m_displays[m_currentMonitorIndex].deviceName, parameters);
    emit parametersChanged();
    applyCurrent();
}

bool BrightnessController::applyCurrent()
{
    if (m_displays.isEmpty()) {
        return false;
    }
    const DisplayInfo &display = m_displays[m_currentMonitorIndex];
    const DisplayParameters parameters = currentParameters();
    const bool applied = DisplayService::apply(display.deviceName, parameters);
    if (applied) {
        m_modifiedDisplays.insert(display.deviceName);
    }
    setStatus(QStringLiteral("%1 · Gamma %2 · 亮度 %3 · 对比度 %4%5")
                  .arg(display.label)
                  .arg(parameters.gamma, 0, 'f', 2)
                  .arg(parameters.brightness, 0, 'f', 0)
                  .arg(parameters.contrast, 0, 'f', 2)
                  .arg(applied ? QString() : QStringLiteral(" · 未生效（驱动不支持或超出系统允许范围）")));
    return applied;
}

void BrightnessController::applyProfile(const QString &name, const QString &statusTemplate)
{
    const QJsonObject profile = profileObject(name);
    if (profile.isEmpty() || m_displays.isEmpty()) {
        return;
    }
    m_selectedProfile = name;
    m_selectedHotkey = profile.value(QStringLiteral("_hotkey")).toString();
    m_parameters.insert(m_displays[m_currentMonitorIndex].deviceName,
                        parametersFromJson(profile));
    const bool applied = applyCurrent();
    emit selectedProfileChanged();
    emit selectedHotkeyChanged();
    emit parametersChanged();
    setStatus(statusTemplate.arg(name)
              + (applied ? QString() : QStringLiteral(" · 未生效（驱动不支持或超出系统允许范围）")));
}

void BrightnessController::applyProfileFromHotkey(const QString &name)
{
    applyProfile(name, QStringLiteral("热键已应用方案“%1”"));
}

void BrightnessController::ensureConfigurationDefaults()
{
    QJsonObject profilesObject = m_configuration.value(QStringLiteral("profiles")).toObject();
    if (!profilesObject.contains(QString::fromLatin1(builtInProfile))) {
        profilesObject.insert(QString::fromLatin1(builtInProfile),
                              parametersToJson(DisplayParameters{}));
    }
    m_configuration.insert(QStringLiteral("profiles"), profilesObject);
    // Temporary display adjustments no longer persist between sessions.
    m_configuration.remove(QStringLiteral("last_params"));
    if (!m_configuration.contains(QStringLiteral("theme"))) {
        m_configuration.insert(QStringLiteral("theme"), QStringLiteral("light"));
    }
    if (!m_configuration.contains(QStringLiteral("disclaimer_ack"))) {
        m_configuration.insert(QStringLiteral("disclaimer_ack"), false);
    }
}

void BrightnessController::loadDisplayParameters()
{
    for (const DisplayInfo &display : m_displays) {
        const DisplayParameters parameters;
        m_parameters.insert(display.deviceName, parameters);
    }
}

void BrightnessController::refreshDisplays()
{
    const QString currentDevice = m_displays.isEmpty()
        ? QString() : m_displays[m_currentMonitorIndex].deviceName;
    const QHash<QString, DisplayParameters> previous = m_parameters;
    m_displays = DisplayService::enumerateDisplays();
    m_parameters.clear();
    m_currentMonitorIndex = 0;
    for (qsizetype index = 0; index < m_displays.size(); ++index) {
        // A reconnected display starts with the system default ramp.
        const QString &device = m_displays[index].deviceName;
        m_parameters.insert(device, previous.value(device, DisplayParameters{}));
        if (device == currentDevice) {
            m_currentMonitorIndex = int(index);
        }
    }
    emit monitorsChanged();
    emit currentMonitorIndexChanged();
    emit parametersChanged();
    setStatus(m_displays.isEmpty()
                  ? QStringLiteral("未检测到可调节的显示器")
                  : QStringLiteral("显示器已变化 · 当前：%1")
                        .arg(m_displays[m_currentMonitorIndex].label));
}

void BrightnessController::refreshProfiles()
{
    const QJsonObject object = m_configuration.value(QStringLiteral("profiles")).toObject();
    QStringList names = object.keys();
    names.removeAll(QString::fromLatin1(builtInProfile));
    names.sort(Qt::CaseInsensitive);
    names.prepend(QString::fromLatin1(builtInProfile));
    if (m_profiles != names) {
        m_profiles = names;
        emit profilesChanged();
    }
    emit profileHotkeysChanged();
}

void BrightnessController::restoreHotkeys()
{
    m_hotkeys.unregisterAll();
    QStringList failures;
    const QJsonObject profilesObject =
        m_configuration.value(QStringLiteral("profiles")).toObject();
    for (auto iterator = profilesObject.constBegin();
         iterator != profilesObject.constEnd(); ++iterator) {
        const QString hotkey = iterator.value().toObject()
                                   .value(QStringLiteral("_hotkey")).toString();
        if (hotkey.isEmpty()) {
            continue;
        }
        QString error;
        if (!m_hotkeys.registerHotkey(iterator.key(), hotkey, &error)) {
            failures.append(QStringLiteral("%1：%2").arg(iterator.key(), error));
        }
    }
    if (!failures.isEmpty()) {
        setStatus(QStringLiteral("部分热键绑定失败 · %1").arg(failures.join(QStringLiteral("；"))));
    }
}

void BrightnessController::restoreDefaultsOnExit()
{
    m_hotkeys.unregisterAll();
    // Startup and shutdown should not reset a display the user never adjusted here.
    for (const auto &device : m_modifiedDisplays) {
        DisplayService::reset(device);
    }
}

bool BrightnessController::persist()
{
    if (m_store.save(m_configuration)) {
        return true;
    }
    setStatus(QStringLiteral("无法写入配置文件，更改仅在本次运行有效：%1")
                  .arg(QDir::toNativeSeparators(m_store.path())));
    return false;
}

void BrightnessController::setStatus(const QString &text)
{
    if (m_statusText == text) {
        return;
    }
    m_statusText = text;
    emit statusTextChanged();
}

QJsonObject BrightnessController::profileObject(const QString &name) const
{
    return m_configuration.value(QStringLiteral("profiles"))
        .toObject().value(name).toObject();
}
