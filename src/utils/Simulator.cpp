#include "Simulator.h"
#include <QDebug>
#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#elif defined(Q_OS_MAC)
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#endif

namespace ClipBridge {

void Simulator::simulatePaste(const QString &keySequence)
{
    // Parse key sequence: split by '+', normalize to uppercase
    bool ctrl = false, shift = false, alt = false, meta = false;
    QString mainKeyStr;

    QStringList parts = keySequence.split('+', Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        QString p = part.trimmed().toUpper();
        if (p == "CTRL" || p == "CONTROL") {
            ctrl = true;
        } else if (p == "SHIFT") {
            shift = true;
        } else if (p == "ALT") {
            alt = true;
        } else if (p == "CMD" || p == "META" || p == "WIN") {
            meta = true;
        } else {
            mainKeyStr = p;
        }
    }

    if (mainKeyStr.isEmpty()) {
        return;
    }

    qDebug() << "Simulator::simulatePaste() called! keySequence:" << keySequence;
#ifdef Q_OS_WIN
    // Map key name to virtual key code
    int vk = 0;
    if (mainKeyStr == "V")          vk = 'V';
    else if (mainKeyStr == "INSERT") vk = VK_INSERT;
    else return;

    // Wait for all modifier keys to be released
    int maxWait = 50;
    for (int i = 0; i < maxWait; ++i) {
        if (!((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)
              || (GetKeyState(VK_SHIFT) & 0x8000) || (GetKeyState(VK_LWIN) & 0x8000)
              || (GetKeyState(VK_RWIN) & 0x8000) || (GetKeyState(vk) & 0x8000)))
            break;
        Sleep(10);
    }

    // Press modifiers
    if (ctrl)  keybd_event(VK_CONTROL, 0, 0, 0);
    if (shift) keybd_event(VK_SHIFT,   0, 0, 0);
    if (alt)   keybd_event(VK_MENU,    0, 0, 0);
    if (meta)  keybd_event(VK_LWIN,    0, 0, 0);

    // Press and release the main key
    keybd_event(vk, 0, 0, 0);
    keybd_event(vk, 0, KEYEVENTF_KEYUP, 0);

    // Release modifiers in reverse order
    if (meta)  keybd_event(VK_LWIN,    0, KEYEVENTF_KEYUP, 0);
    if (alt)   keybd_event(VK_MENU,    0, KEYEVENTF_KEYUP, 0);
    if (shift) keybd_event(VK_SHIFT,   0, KEYEVENTF_KEYUP, 0);
    if (ctrl)  keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);

#elif defined(Q_OS_LINUX)
    // Map key name to X11 keysym
    KeySym ks = 0;
    if (mainKeyStr == "V")       ks = XK_v;
    else if (mainKeyStr == "INSERT") ks = XK_Insert;
    else return;

    Display *dpy = XOpenDisplay(nullptr);
    if (dpy) {
        // Release any stuck modifiers first
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), False, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Shift_L),   False, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Alt_L),     False, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Meta_L),    False, 0);
        XFlush(dpy);
        usleep(50000);

        // Press modifiers
        if (ctrl)  XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), True, 0);
        if (shift) XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Shift_L),   True, 0);
        if (alt)   XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Alt_L),     True, 0);
        if (meta)  XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Meta_L),    True, 0);

        // Press and release main key
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, ks), True, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, ks), False, 0);

        // Release modifiers
        if (meta)  XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Meta_L),    False, 0);
        if (alt)   XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Alt_L),     False, 0);
        if (shift) XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Shift_L),   False, 0);
        if (ctrl)  XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), False, 0);
        XFlush(dpy);
        XCloseDisplay(dpy);
    }
#elif defined(Q_OS_MAC)
    // Map key name to macOS virtual key code
    int macVk = 0;
    if (mainKeyStr == "V")       macVk = kVK_ANSI_V;
    else if (mainKeyStr == "INSERT") macVk = kVK_Help;
    else return;

    // Wait for all modifier keys to be released
    int maxWait = 50;
    for (int i = 0; i < maxWait; ++i) {
        if (!(CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, kVK_Command)
              || CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, kVK_Option)
              || CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, kVK_Control)
              || CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, kVK_Shift)))
            break;
        usleep(10000);
    }

    CGEventSourceRef src = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    if (src) {
        auto postKey = [src](int code, bool down) {
            CGEventRef e = CGEventCreateKeyboardEvent(src, code, down);
            if (e) { CGEventPost(kCGHIDEventTap, e); CFRelease(e); }
        };

        // Press modifiers
        if (ctrl || meta)  postKey(kVK_Command, true);  // Cmd or Ctrl both map to Command
        if (shift) postKey(kVK_Shift,   true);
        if (alt)   postKey(kVK_Option,  true);
        usleep(10000);

        // Press and release main key
        postKey(macVk, true);
        usleep(10000);
        postKey(macVk, false);
        usleep(10000);

        // Release modifiers (reverse order)
        if (alt)   postKey(kVK_Option,  false);
        if (shift) postKey(kVK_Shift,   false);
        if (ctrl || meta)  postKey(kVK_Command, false);

        CFRelease(src);
    }
#endif
}

void Simulator::simulateEnter()
{
#ifdef Q_OS_WIN
    // 等待所有修饰键松开
    int maxWait = 50; // 最多等 500ms
    int waitCount = 0;
    while (waitCount < maxWait) {
        bool ctrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        bool altDown = (GetKeyState(VK_MENU) & 0x8000) != 0;
        bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        bool winDown = (GetKeyState(VK_LWIN) & 0x8000) != 0 || (GetKeyState(VK_RWIN) & 0x8000) != 0;

        if (!ctrlDown && !altDown && !shiftDown && !winDown) {
            break;
        }
        Sleep(10);
        waitCount++;
    }

    keybd_event(VK_RETURN, 0, 0, 0);
    keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
#elif defined(Q_OS_LINUX)
    Display *dpy = XOpenDisplay(nullptr);
    if (dpy) {
        // 先发送松开事件
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), False, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Shift_L), False, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Alt_L), False, 0);
        XFlush(dpy);
        usleep(50000);

        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Return), True, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Return), False, 0);
        XFlush(dpy);
        XCloseDisplay(dpy);
    }
#elif defined(Q_OS_MAC)
    // 等待所有修饰键松开
    int maxWait = 50; // 最多等 500ms
    int waitCount = 0;
    while (waitCount < maxWait) {
        bool cmdDown = CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, kVK_Command);
        bool optDown = CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, kVK_Option);
        bool ctrlDown = CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, kVK_Control);
        bool shiftDown = CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, kVK_Shift);

        if (!cmdDown && !optDown && !ctrlDown && !shiftDown) {
            break;
        }
        usleep(10000);
        waitCount++;
    }

    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    if (source) {
        CGEventRef enterDown = CGEventCreateKeyboardEvent(source, kVK_Return, true);
        CGEventPost(kCGHIDEventTap, enterDown);
        CFRelease(enterDown);

        CGEventRef enterUp = CGEventCreateKeyboardEvent(source, kVK_Return, false);
        CGEventPost(kCGHIDEventTap, enterUp);
        CFRelease(enterUp);

        CFRelease(source);
    }
#endif
}

}