#include "hotkey.h"
#include <QGuiApplication>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <QX11Info>
#include <QTimer>
#elif defined(Q_OS_MAC)
#include <ApplicationServices/ApplicationServices.h>
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
#elif defined(Q_OS_MAC)
    , m_eventMonitor(nullptr)
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
    qDebug() << "Hotkey: Linux/X11 support is limited in this version";
    m_registered = false;
#elif defined(Q_OS_MAC)
    qDebug() << "Hotkey: macOS support requires accessibility permissions";
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
    // 清理 X11 资源
#elif defined(Q_OS_MAC)
    // 清理 macOS 资源
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
#endif

    return false;
}

} // namespace ClipBridge
