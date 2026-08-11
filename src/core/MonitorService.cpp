#include "MonitorService.h"
#include "ActionManager.h"
#include "ClipboardHelper.h"
#include <QApplication>
#include <QClipboard>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace ClipBridge {

// ---- RAII processing guard ------------------------------------------------
namespace {

class ProcessingGuard {
public:
    explicit ProcessingGuard(bool &flag) : m_flag(flag) { m_flag = true; }
    ~ProcessingGuard() { m_flag = false; }
    ProcessingGuard(const ProcessingGuard &) = delete;
    ProcessingGuard &operator=(const ProcessingGuard &) = delete;
private:
    bool &m_flag;
};

} // anonymous namespace

// ---- Low-level keyboard hook (paste detection without consuming) -----------

#ifdef Q_OS_WIN
#ifndef LLKHF_INJECTED
#define LLKHF_INJECTED 0x10
#endif
static HHOOK g_pasteHook = nullptr;

static LRESULT CALLBACK PasteHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *kb = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

        // Skip injected events — our own simulated pastes must not re-trigger
        if (kb->flags & LLKHF_INJECTED)
            return CallNextHookEx(nullptr, nCode, wParam, lParam);

        bool ctrl  = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        bool shift = (GetAsyncKeyState(VK_SHIFT)   & 0x8000) != 0;

        bool isPaste = false;
        if (ctrl && !shift && kb->vkCode == 'V')        isPaste = true;
        else if (ctrl && shift && kb->vkCode == 'V')    isPaste = true;
        else if (shift && kb->vkCode == VK_INSERT)      isPaste = true;

        if (isPaste)
            MonitorService::notifyPasteDetected();
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
#endif

static MonitorService *s_instance = nullptr;

void MonitorService::notifyPasteDetected()
{
    // Called from hook context on main thread — safely defer via signal
    if (s_instance)
        emit s_instance->pasteDetected();
}

// ---- MonitorService -------------------------------------------------------

MonitorService::MonitorService(ActionManager *actionManager, QObject *parent)
    : QObject(parent), m_actionManager(actionManager)
{
    s_instance = this;
    connect(this, &MonitorService::pasteDetected,
            this, &MonitorService::onPasteDetected);
}

MonitorService::~MonitorService()
{
    removePasteHook();
    if (s_instance == this)
        s_instance = nullptr;
}

bool MonitorService::isEnabled() const { return m_enabled; }

void MonitorService::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        if (enabled)
            installPasteHook();
        else
            removePasteHook();
        emit enabledChanged(m_enabled);
        qDebug() << "Monitor mode" << (enabled ? "enabled" : "disabled");
    }
}

void MonitorService::enterIgnoreScope()
{
    ++m_ignoreCount;
}

void MonitorService::leaveIgnoreScope()
{
    if (m_ignoreCount > 0)
        --m_ignoreCount;
}

void MonitorService::updateConfig(const AppConfig::MonitorConfig &monitorConfig,
                                  const QString &pasteKey)
{
    bool wasEnabled = m_enabled;
    QString oldPasteKey = m_pasteKey;

    m_config = monitorConfig;
    m_pasteKey = pasteKey;

    if (wasEnabled && m_pasteKey != oldPasteKey) {
        removePasteHook();
        installPasteHook();
    }
}

void MonitorService::installPasteHook()
{
#ifdef Q_OS_WIN
    if (g_pasteHook) return;
    g_pasteHook = SetWindowsHookExW(WH_KEYBOARD_LL,
        PasteHookProc, GetModuleHandleW(nullptr), 0);
    if (g_pasteHook)
        qDebug() << "Paste hook installed";
    else
        qWarning() << "Failed to install paste hook:" << GetLastError();
#endif
}

void MonitorService::removePasteHook()
{
#ifdef Q_OS_WIN
    if (g_pasteHook) {
        UnhookWindowsHookEx(g_pasteHook);
        g_pasteHook = nullptr;
        qDebug() << "Paste hook removed";
    }
#endif
}

void MonitorService::onPasteDetected()
{
    if (!m_enabled || m_ignoreCount > 0 || m_processing)
        return;

    ProcessingGuard guard{m_processing};
    tryAutoActions();
}

void MonitorService::tryAutoActions()
{
    QString type = ClipboardHelper::detectContentType();

    for (const QString &action : m_config.autoActions) {
        if (!ActionManager::canHandleAction(action, type))
            continue;

        // long_text has an additional threshold gate:
        //   0  → always match (threshold check disabled)
        //   >0 → match only when clipboard text exceeds this many characters
        if (action == "clipboard_long_text"
            && m_config.longTextThreshold > 0
            && !ClipboardHelper::isTextLong(m_config.longTextThreshold))
            continue;

        AppConfig::Behavior behavior;
        behavior.autoPaste = true;
        behavior.autoSubmit = false;
        m_actionManager->run(action, behavior);
        break;
    }
}

} // namespace ClipBridge
