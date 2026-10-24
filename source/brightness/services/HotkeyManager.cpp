#include "brightness/services/HotkeyManager.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <windows.h>

HotkeyManager::HotkeyManager(QObject *parent)
    : QObject(parent)
{
    QCoreApplication::instance()->installNativeEventFilter(this);
}

HotkeyManager::~HotkeyManager()
{
    unregisterAll();
    if (QCoreApplication::instance()) {
        QCoreApplication::instance()->removeNativeEventFilter(this);
    }
}

bool HotkeyManager::registerHotkey(const QString &owner, const QString &sequence,
                                   QString *errorMessage)
{
    ParsedHotkey hotkey;
    if (!parse(sequence, &hotkey, errorMessage)) {
        return false;
    }

    const quint64 signature =
        (quint64(hotkey.modifiers) << 32) | hotkey.virtualKeys.constFirst();
    for (auto iterator = m_signatureByOwner.cbegin();
         iterator != m_signatureByOwner.cend(); ++iterator) {
        if (iterator.key() != owner && iterator.value() == signature) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("快捷键已被方案“%1”使用").arg(iterator.key());
            }
            return false;
        }
    }

    unregisterOwner(owner);
    QList<int> registeredIds;
    for (const unsigned int virtualKey : hotkey.virtualKeys) {
        const int id = m_nextId++;
        const UINT modifiers = hotkey.modifiers | MOD_NOREPEAT;
        if (!RegisterHotKey(nullptr, id, modifiers, virtualKey)) {
            // Read the error before UnregisterHotKey can overwrite it.
            const DWORD code = GetLastError();
            for (const int registeredId : registeredIds) {
                UnregisterHotKey(nullptr, registeredId);
                m_ownerById.remove(registeredId);
            }
            if (errorMessage) {
                *errorMessage = code == ERROR_HOTKEY_ALREADY_REGISTERED
                    ? QStringLiteral("快捷键已被其他程序占用")
                    : QStringLiteral("快捷键注册失败（错误 %1）").arg(code);
            }
            return false;
        }
        registeredIds.append(id);
        m_ownerById.insert(id, owner);
    }

    m_idsByOwner.insert(owner, registeredIds);
    m_signatureByOwner.insert(owner, signature);
    return true;
}

void HotkeyManager::unregisterOwner(const QString &owner)
{
    const QList<int> ids = m_idsByOwner.take(owner);
    for (const int id : ids) {
        UnregisterHotKey(nullptr, id);
        m_ownerById.remove(id);
    }
    m_signatureByOwner.remove(owner);
}

void HotkeyManager::unregisterAll()
{
    const QList<int> ids = m_ownerById.keys();
    for (const int id : ids) {
        UnregisterHotKey(nullptr, id);
    }
    m_ownerById.clear();
    m_idsByOwner.clear();
    m_signatureByOwner.clear();
}

bool HotkeyManager::nativeEventFilter(const QByteArray &eventType, void *message,
                                      qintptr *result)
{
    if (eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG") {
        return false;
    }

    const auto *nativeMessage = static_cast<MSG *>(message);
    if (nativeMessage && nativeMessage->message == WM_HOTKEY) {
        const int id = static_cast<int>(nativeMessage->wParam);
        const auto iterator = m_ownerById.constFind(id);
        if (iterator != m_ownerById.cend()) {
            const QString owner = iterator.value();
            if (result)
                *result = 0;
            QMetaObject::invokeMethod(this, [this, owner] { emit activated(owner); },
                                      Qt::QueuedConnection);
            return true;
        }
    }
    return false;
}

bool HotkeyManager::parse(const QString &sequence, ParsedHotkey *parsed,
                          QString *errorMessage)
{
    if (!parsed) {
        return false;
    }

    const QStringList tokens = sequence.toLower().split(
        QRegularExpression(QStringLiteral("\\s*\\+\\s*")), Qt::SkipEmptyParts);
    if (tokens.size() < 2) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("请使用组合键，例如 Ctrl+Alt+1");
        }
        return false;
    }

    ParsedHotkey result;
    const QString keyToken = tokens.constLast();
    for (qsizetype index = 0; index < tokens.size() - 1; ++index) {
        const QString &token = tokens[index];
        if (token == QStringLiteral("ctrl") || token == QStringLiteral("control")) {
            result.modifiers |= MOD_CONTROL;
        } else if (token == QStringLiteral("alt")) {
            result.modifiers |= MOD_ALT;
        } else if (token == QStringLiteral("shift")) {
            result.modifiers |= MOD_SHIFT;
        } else if (token == QStringLiteral("win") || token == QStringLiteral("meta")) {
            result.modifiers |= MOD_WIN;
        } else {
            if (errorMessage) {
                *errorMessage = QStringLiteral("不支持的修饰键：%1").arg(token);
            }
            return false;
        }
    }

    if (keyToken.size() == 1 && keyToken[0].isLetter()) {
        result.virtualKeys.append(static_cast<unsigned int>(keyToken[0].toUpper().unicode()));
    } else if (keyToken.size() == 1 && keyToken[0].isDigit()) {
        const int digit = keyToken[0].digitValue();
        result.virtualKeys.append(static_cast<unsigned int>('0' + digit));
        result.virtualKeys.append(static_cast<unsigned int>(VK_NUMPAD0 + digit));
    } else {
        static const QRegularExpression functionKey(QStringLiteral("^f([1-9]|1[0-9]|2[0-4])$"));
        const QRegularExpressionMatch match = functionKey.match(keyToken);
        if (match.hasMatch()) {
            result.virtualKeys.append(static_cast<unsigned int>(VK_F1 + match.captured(1).toInt() - 1));
        } else {
            if (errorMessage) {
                *errorMessage = QStringLiteral("仅支持字母、数字和 F1–F24");
            }
            return false;
        }
    }

    parsed->modifiers = result.modifiers;
    parsed->virtualKeys = result.virtualKeys;
    return true;
}
