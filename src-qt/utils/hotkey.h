#pragma once

#include <QObject>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>

/**
 * @brief 全局热键支持（Windows 专用）
 */
class Hotkey : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit Hotkey(const QKeySequence &keySequence, bool autoRegister = false, QObject *parent = nullptr);
    ~Hotkey() override;

    bool isRegistered() const { return m_registered; }
    bool registerHotkey();
    void unregisterHotkey();

    QKeySequence keySequence() const { return m_keySequence; }

signals:
    void activated();

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

private:
    QKeySequence m_keySequence;
    bool m_registered;
    int m_hotkeyId;

    static int m_nextId;
};
