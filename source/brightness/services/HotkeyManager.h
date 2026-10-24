#pragma once

#include <QAbstractNativeEventFilter>
#include <QHash>
#include <QList>
#include <QObject>
#include <QString>

class HotkeyManager final : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit HotkeyManager(QObject *parent = nullptr);
    ~HotkeyManager() override;

    bool registerHotkey(const QString &owner, const QString &sequence,
                        QString *errorMessage = nullptr);
    void unregisterOwner(const QString &owner);
    void unregisterAll();

    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           qintptr *result) override;

signals:
    void activated(const QString &owner);

private:
    struct ParsedHotkey {
        unsigned int modifiers = 0;
        QList<unsigned int> virtualKeys;
    };

    static bool parse(const QString &sequence, ParsedHotkey *parsed,
                      QString *errorMessage);

    int m_nextId = 0x4700;
    QHash<int, QString> m_ownerById;
    QHash<QString, QList<int>> m_idsByOwner;
    QHash<QString, quint64> m_signatureByOwner;
};

