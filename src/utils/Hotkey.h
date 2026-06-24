#pragma once

#include <QObject>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>
#include <QtGlobal>
#include <QString>

#ifdef Q_OS_MAC
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#endif

#include "core/AppConfig.h"

namespace ClipBridge {

/**
 * @brief 全局热键支持
 *
 * 平台支持:
 * - Windows: ✅ 完整实现
 * - Linux (X11): ✅ 完整实现
 * - macOS: ✅ 完整实现 (需要辅助功能权限)
 */
class Hotkey : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit Hotkey(const QString &action, const QKeySequence &keySequence, const AppConfig::Behavior &behavior, bool autoRegister = false, QObject *parent = nullptr);
    ~Hotkey();

    bool isRegistered() const { return m_registered; }
    bool registerHotkey();
    void unregisterHotkey();

    QKeySequence keySequence() const { return m_keySequence; }
    QString action() const { return m_action; }
    AppConfig::Behavior behavior() const { return m_behavior; }

signals:
    void activated(const QString &action, const AppConfig::Behavior &behavior);

private slots:
    void trigger();

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif

private:
    QString m_action;
    AppConfig::Behavior m_behavior;
    QKeySequence m_keySequence;
    bool m_registered;
    int m_hotkeyId;

    static int m_nextId;

    // 所有热键实例列表（线程安全）
    static QList<Hotkey*> s_allHotkeys;
    static QMutex s_hotkeyMutex;

#ifdef Q_OS_WIN
    // Windows 专用
#elif defined(Q_OS_LINUX)
    // Linux 专用
    quint32 m_x11Keycode;
    quint32 m_x11Modifiers;
#elif defined(Q_OS_MAC)
    // macOS 专用
    void *m_eventTap;
    void *m_runLoopSource;
    CGKeyCode m_keycode;
    CGEventFlags m_flags;
    static CGEventRef macKeyEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon);
#endif
};

} // namespace ClipBridge
