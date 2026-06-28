#include "Hotkey.h"
#include <QGuiApplication>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <QX11Info>
#elif defined(Q_OS_MAC)
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#endif

namespace ClipBridge {

namespace {
    // Global event filter to handle all hotkeys
    class HotkeyEventFilter : public QAbstractNativeEventFilter {
    public:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
#else
        bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override
#endif
        {
            return Hotkey::handleGlobalEvent(eventType, message, result);
        }
    };

    HotkeyEventFilter *g_hotkeyEventFilter = nullptr;
}

int Hotkey::m_nextId = 1;
QList<Hotkey*> Hotkey::s_allHotkeys;
QMutex Hotkey::s_hotkeyMutex;

Hotkey::Hotkey(const QString &action, const QKeySequence &keySequence, const AppConfig::Behavior &behavior, bool autoRegister, QObject *parent)
    : QObject(parent)
    , m_action(action)
    , m_behavior(behavior)
    , m_keySequence(keySequence)
    , m_registered(false)
    , m_hotkeyId(m_nextId++)
#ifdef Q_OS_LINUX
    , m_x11Keycode(0)
    , m_x11Modifiers(0)
#elif defined(Q_OS_MAC)
    , m_eventTap(nullptr)
    , m_runLoopSource(nullptr)
    , m_keycode(0)
    , m_flags(0)
#endif
{
    // Install global event filter once
    if (!g_hotkeyEventFilter) {
        g_hotkeyEventFilter = new HotkeyEventFilter();
        qApp->installNativeEventFilter(g_hotkeyEventFilter);
    }

    // 添加到全局列表
    {
        QMutexLocker locker(&s_hotkeyMutex);
        s_allHotkeys.append(this);
    }

    if (autoRegister) {
        registerHotkey();
    }
}

Hotkey::~Hotkey()
{
    // 先从全局列表移除，这样回调就不会访问这个对象了
    {
        QMutexLocker locker(&s_hotkeyMutex);
        s_allHotkeys.removeAll(this);
    }

    unregisterHotkey();
}

void Hotkey::trigger()
{
    emit activated(m_action, m_behavior);
}

bool Hotkey::handleGlobalEvent(const QByteArray &eventType, void *message, void *result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);

#ifdef Q_OS_WIN
    MSG *msg = static_cast<MSG *>(message);
    if (msg->message == WM_HOTKEY) {
        // Find the hotkey that matches this ID
        QMutexLocker locker(&s_hotkeyMutex);
        for (Hotkey *hotkey : s_allHotkeys) {
            if (hotkey->m_hotkeyId == static_cast<int>(msg->wParam)) {
                emit hotkey->activated(hotkey->m_action, hotkey->m_behavior);
                return true;
            }
        }
    }
#elif defined(Q_OS_LINUX)
    if (QX11Info::isPlatformX11()) {
        Display *dpy = QX11Info::display();
        if (dpy) {
            while (XPending(dpy)) {
                XEvent event;
                XNextEvent(dpy, &event);
                if (event.type == KeyPress) {
                    QMutexLocker locker(&s_hotkeyMutex);
                    for (Hotkey *hotkey : s_allHotkeys) {
                        if (hotkey->m_registered && event.xkey.keycode == hotkey->m_x11Keycode) {
                            unsigned int state = event.xkey.state & (ControlMask | ShiftMask | Mod1Mask | Mod4Mask);
                            if (state == hotkey->m_x11Modifiers) {
                                emit hotkey->activated(hotkey->m_action, hotkey->m_behavior);
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
#endif
    return false;
}

#ifdef Q_OS_MAC
CGEventRef Hotkey::macKeyEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
    Q_UNUSED(proxy);

    if (type == kCGEventKeyDown && refcon) {
        Hotkey *hotkey = static_cast<Hotkey*>(refcon);

        // 关键安全检查：先验证 hotkey 是否还在全局列表里
        bool isValid = false;
        {
            QMutexLocker locker(&s_hotkeyMutex);
            isValid = s_allHotkeys.contains(hotkey);
        }

        if (!isValid) {
            return event;
        }

        CGKeyCode keycode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
        CGEventFlags flags = CGEventGetFlags(event) & (kCGEventFlagMaskControl | kCGEventFlagMaskShift | kCGEventFlagMaskAlternate | kCGEventFlagMaskCommand);

        if (keycode == hotkey->m_keycode && flags == hotkey->m_flags) {
            // 安全发射信号，用 QueuedConnection 确保在主线程执行
            QMetaObject::invokeMethod(hotkey, "trigger", Qt::QueuedConnection);
        }
    }

    return event;
}
#endif

bool Hotkey::registerHotkey()
{
    if (m_registered) {
        return true;
    }

#ifdef Q_OS_WIN
    if (m_keySequence.isEmpty()) {
        return false;
    }

    QString str = m_keySequence.toString(QKeySequence::PortableText);
    QStringList parts = str.split('+');

    UINT fsModifiers = 0;
    UINT key = 0;

    for (const QString &part : parts) {
        QString p = part.trimmed().toLower();
        if (p == "ctrl" || p == "control") {
            fsModifiers |= MOD_CONTROL;
        } else if (p == "alt") {
            fsModifiers |= MOD_ALT;
        } else if (p == "shift") {
            fsModifiers |= MOD_SHIFT;
        } else if (p == "meta" || p == "win") {
            fsModifiers |= MOD_WIN;
        } else if (!p.isEmpty()) {
            QKeySequence ks(p);
            if (!ks.isEmpty()) {
                key = ks[0] & ~Qt::KeyboardModifierMask;
            }
        }
    }

    m_registered = RegisterHotKey(NULL, m_hotkeyId, fsModifiers, key) != FALSE;
#elif defined(Q_OS_LINUX)
    if (!QX11Info::isPlatformX11()) {
        qWarning() << "Hotkey: Not running on X11";
        return false;
    }

    if (m_keySequence.isEmpty()) {
        return false;
    }

    Display *dpy = QX11Info::display();
    if (!dpy) {
        return false;
    }

    QString str = m_keySequence.toString(QKeySequence::PortableText);
    QStringList parts = str.split('+');

    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    int key = 0;

    for (const QString &part : parts) {
        QString p = part.trimmed().toLower();
        if (p == "ctrl" || p == "control") {
            modifiers |= Qt::ControlModifier;
        } else if (p == "alt") {
            modifiers |= Qt::AltModifier;
        } else if (p == "shift") {
            modifiers |= Qt::ShiftModifier;
        } else if (p == "meta" || p == "super" || p == "win") {
            modifiers |= Qt::MetaModifier;
        } else if (!p.isEmpty()) {
            QKeySequence ks(p);
            if (!ks.isEmpty()) {
                key = ks[0] & ~Qt::KeyboardModifierMask;
            }
        }
    }

    int keysym = 0;
    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        keysym = XK_a + (key - Qt::Key_A);
    } else if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        keysym = XK_0 + (key - Qt::Key_0);
    } else if (key == Qt::Key_F1) keysym = XK_F1;
    else if (key == Qt::Key_F2) keysym = XK_F2;
    else if (key == Qt::Key_F3) keysym = XK_F3;
    else if (key == Qt::Key_F4) keysym = XK_F4;
    else if (key == Qt::Key_F5) keysym = XK_F5;
    else if (key == Qt::Key_F6) keysym = XK_F6;
    else if (key == Qt::Key_F7) keysym = XK_F7;
    else if (key == Qt::Key_F8) keysym = XK_F8;
    else if (key == Qt::Key_F9) keysym = XK_F9;
    else if (key == Qt::Key_F10) keysym = XK_F10;
    else if (key == Qt::Key_F11) keysym = XK_F11;
    else if (key == Qt::Key_F12) keysym = XK_F12;
    else if (key == Qt::Key_I) keysym = XK_I;
    else if (key == Qt::Key_J) keysym = XK_J;
    else if (key == Qt::Key_K) keysym = XK_K;

    if (keysym == 0) {
        qWarning() << "Hotkey: Unsupported key";
        return false;
    }

    m_x11Keycode = XKeysymToKeycode(dpy, keysym);
    m_x11Modifiers = 0;
    if (modifiers & Qt::ControlModifier) m_x11Modifiers |= ControlMask;
    if (modifiers & Qt::ShiftModifier) m_x11Modifiers |= ShiftMask;
    if (modifiers & Qt::AltModifier) m_x11Modifiers |= Mod1Mask;
    if (modifiers & Qt::MetaModifier) m_x11Modifiers |= Mod4Mask;

    Window root = DefaultRootWindow(dpy);
    XGrabKey(dpy, m_x11Keycode, m_x11Modifiers, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, m_x11Keycode, m_x11Modifiers | LockMask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, m_x11Keycode, m_x11Modifiers | Mod2Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, m_x11Keycode, m_x11Modifiers | LockMask | Mod2Mask, root, True, GrabModeAsync, GrabModeAsync);

    m_registered = true;
#elif defined(Q_OS_MAC)
    if (m_keySequence.isEmpty()) {
        return false;
    }

    // 检查辅助功能权限
    bool hasPermission = AXIsProcessTrusted();
    if (!hasPermission) {
        qWarning() << "Hotkey: Accessibility permissions required!";
        qWarning() << "Please enable in: System Settings > Privacy & Security > Accessibility";

        CFDictionaryRef options = CFDictionaryCreate(
            kCFAllocatorDefault,
            (const void**)&kAXTrustedCheckOptionPrompt,
            (const void**)&kCFBooleanTrue,
            1,
            nullptr, nullptr);
        AXIsProcessTrustedWithOptions(options);
        CFRelease(options);
        return false;
    }

    QString str = m_keySequence.toString(QKeySequence::PortableText);
    QStringList parts = str.split('+');

    QHash<QString, CGKeyCode> keyMap = {
        {"a", 0x00}, {"b", 0x0B}, {"c", 0x08}, {"d", 0x02}, {"e", 0x0E},
        {"f", 0x03}, {"g", 0x05}, {"h", 0x04}, {"i", 0x22}, {"j", 0x26},
        {"k", 0x28}, {"l", 0x25}, {"m", 0x2E}, {"n", 0x2D}, {"o", 0x1F},
        {"p", 0x23}, {"q", 0x0C}, {"r", 0x0F}, {"s", 0x01}, {"t", 0x11},
        {"u", 0x20}, {"v", 0x09}, {"w", 0x0D}, {"x", 0x07}, {"y", 0x10},
        {"z", 0x06},
        {"0", 0x1D}, {"1", 0x12}, {"2", 0x13}, {"3", 0x14}, {"4", 0x15},
        {"5", 0x17}, {"6", 0x16}, {"7", 0x1A}, {"8", 0x1C}, {"9", 0x19},
        {"f1", 0x7A}, {"f2", 0x78}, {"f3", 0x63}, {"f4", 0x76}, {"f5", 0x60},
        {"f6", 0x61}, {"f7", 0x62}, {"f8", 0x64}, {"f9", 0x65}, {"f10", 0x6D},
        {"f11", 0x67}, {"f12", 0x6F}
    };

    m_flags = 0;
    m_keycode = 0xFF;

    for (const QString &part : parts) {
        QString p = part.trimmed().toLower();
        if (p == "ctrl" || p == "control") {
            m_flags |= kCGEventFlagMaskControl;
        } else if (p == "alt" || p == "option") {
            m_flags |= kCGEventFlagMaskAlternate;
        } else if (p == "shift") {
            m_flags |= kCGEventFlagMaskShift;
        } else if (p == "cmd" || p == "command" || p == "meta") {
            m_flags |= kCGEventFlagMaskCommand;
        } else if (keyMap.contains(p)) {
            m_keycode = keyMap[p];
        }
    }

    if (m_keycode == 0xFF) {
        qWarning() << "Hotkey: Unsupported key" << m_keySequence.toString();
        return false;
    }

    m_eventTap = CGEventTapCreate(kCGHIDEventTap, kCGHeadInsertEventTap, kCGEventTapOptionListenOnly,
        CGEventMaskBit(kCGEventKeyDown),
        &Hotkey::macKeyEventCallback, this);

    if (!m_eventTap) {
        qWarning() << "Hotkey: Failed to create event tap";
        return false;
    }

    m_runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, (CFMachPortRef)m_eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), (CFRunLoopSourceRef)m_runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable((CFMachPortRef)m_eventTap, true);

    m_registered = true;
#else
    m_registered = false;
#endif

    if (m_registered) {
        qDebug() << "Registered hotkey:" << m_keySequence.toString();
    }

    return m_registered;
}

void Hotkey::unregisterHotkey()
{
    if (!m_registered) {
        return;
    }

#ifdef Q_OS_WIN
    UnregisterHotKey(NULL, m_hotkeyId);
#elif defined(Q_OS_LINUX)
    if (QX11Info::isPlatformX11()) {
        Display *dpy = QX11Info::display();
        if (dpy) {
            Window root = DefaultRootWindow(dpy);
            XUngrabKey(dpy, m_x11Keycode, m_x11Modifiers, root);
            XUngrabKey(dpy, m_x11Keycode, m_x11Modifiers | LockMask, root);
            XUngrabKey(dpy, m_x11Keycode, m_x11Modifiers | Mod2Mask, root);
            XUngrabKey(dpy, m_x11Keycode, m_x11Modifiers | LockMask | Mod2Mask, root);
        }
    }
#elif defined(Q_OS_MAC)
    if (m_eventTap) {
        CGEventTapEnable((CFMachPortRef)m_eventTap, false);
        if (m_runLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(), (CFRunLoopSourceRef)m_runLoopSource, kCFRunLoopCommonModes);
            CFRelease((CFRunLoopSourceRef)m_runLoopSource);
            m_runLoopSource = nullptr;
        }
        CFRelease((CFMachPortRef)m_eventTap);
        m_eventTap = nullptr;
    }
#endif

    m_registered = false;
}

// The global HotkeyEventFilter handles events now, so we don't need per-instance filtering
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool Hotkey::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#else
bool Hotkey::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
#endif
{
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    Q_UNUSED(result);
    return false;
}

} // namespace ClipBridge
