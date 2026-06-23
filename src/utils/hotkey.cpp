#include "hotkey.h"
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
#include <QProcess>
#endif

namespace ClipBridge {

int Hotkey::m_nextId = 1;

Hotkey::Hotkey(const QKeySequence &keySequence, bool autoRegister, QObject *parent)
    : QObject(parent)
    , m_keySequence(keySequence)
    , m_registered(false)
    , m_hotkeyId(m_nextId++)
#ifdef Q_OS_LINUX
    , m_x11Keycode(0)
    , m_x11Modifiers(0)
#endif
{
    qApp->installNativeEventFilter(this);

    if (autoRegister) {
        registerHotkey();
    }
}

Hotkey::~Hotkey()
{
    unregisterHotkey();
    qApp->removeNativeEventFilter(this);
}

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

    m_registered = RegisterHotKey(nullptr, m_hotkeyId, fsModifiers, key) != FALSE;
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

    // 转换为 X11 keysym
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
    else if (key == Qt::Key_I) keysym = XK_i;
    else if (key == Qt::Key_J) keysym = XK_j;
    else if (key == Qt::Key_K) keysym = XK_k;

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
    qDebug() << "Hotkey: macOS requires accessibility permissions, implementation is limited";
    m_registered = false;
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
    UnregisterHotKey(nullptr, m_hotkeyId);
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
#endif

    m_registered = false;
}

bool Hotkey::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);

#ifdef Q_OS_WIN
    MSG *msg = static_cast<MSG *>(message);
    if (msg->message == WM_HOTKEY && msg->wParam == m_hotkeyId) {
        emit activated();
        return true;
    }
#elif defined(Q_OS_LINUX)
    if (QX11Info::isPlatformX11() && m_registered) {
        Display *dpy = QX11Info::display();
        if (dpy) {
            while (XPending(dpy)) {
                XEvent event;
                XNextEvent(dpy, &event);
                if (event.type == KeyPress) {
                    if (event.xkey.keycode == m_x11Keycode) {
                        unsigned int state = event.xkey.state & (ControlMask | ShiftMask | Mod1Mask | Mod4Mask);
                        if (state == m_x11Modifiers) {
                            emit activated();
                            return true;
                        }
                    }
                }
            }
        }
    }
#endif

    return false;
}

} // namespace ClipBridge
