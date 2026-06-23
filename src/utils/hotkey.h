#pragma once

#include <QObject>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>

namespace ClipBridge {

/**
 * @brief 全局热键支持
 *
 * 平台支持:
 * - Windows: ✅ 完整实现
 * - Linux (X11): ✅ 完整实现
 * - macOS: ⚠️ 需要辅助功能权限
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

#ifdef Q_OS_WIN
    // Windows 专用
#elif defined(Q_OS_LINUX)
    // Linux 专用
    quint32 m_x11Keycode;
    quint32 m_x11Modifiers;
#elif defined(Q_OS_MAC)
    // macOS 专用
#endif
};

} // namespace ClipBridge
