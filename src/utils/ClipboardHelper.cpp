#include "ClipboardHelper.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDir>
#include <QStandardPaths>

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

bool ClipboardHelper::hasImage()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    return mimeData && mimeData->hasImage();
}

QImage ClipboardHelper::getImage()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (mimeData && mimeData->hasImage()) {
        return qvariant_cast<QImage>(mimeData->imageData());
    }
    return QImage();
}

bool ClipboardHelper::saveImageToPng(const QImage &image, const QString &path)
{
    if (image.isNull()) {
        return false;
    }

    QFileInfo fileInfo(path);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    return image.save(path, "PNG");
}

QString ClipboardHelper::getText()
{
    return QApplication::clipboard()->text();
}

void ClipboardHelper::setText(const QString &text)
{
    QApplication::clipboard()->setText(text);
}

void ClipboardHelper::simulatePaste()
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
        bool vDown = (GetKeyState('V') & 0x8000) != 0;

        if (!ctrlDown && !altDown && !shiftDown && !winDown && !vDown) {
            break;
        }
        Sleep(10);
        waitCount++;
    }

    keybd_event(VK_CONTROL, 0, 0, 0);
    keybd_event('V', 0, 0, 0);
    keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
#elif defined(Q_OS_LINUX)
    Display *dpy = XOpenDisplay(nullptr);
    if (dpy) {
        // 先发送松开事件
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), False, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Shift_L), False, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Alt_L), False, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_v), False, 0);
        XFlush(dpy);
        usleep(50000);

        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), True, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_v), True, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_v), False, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), False, 0);
        XFlush(dpy);
        XCloseDisplay(dpy);
    }
#elif defined(Q_OS_MAC)
    // // 先发送所有修饰键的松开事件，确保没有卡住
    // CGEventSourceRef flushSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    // if (flushSource) {
    //     CGEventRef cmdUp = CGEventCreateKeyboardEvent(flushSource, kVK_Command, false);
    //     CGEventRef optUp = CGEventCreateKeyboardEvent(flushSource, kVK_Option, false);
    //     CGEventRef ctrlUp = CGEventCreateKeyboardEvent(flushSource, kVK_Control, false);
    //     CGEventRef shiftUp = CGEventCreateKeyboardEvent(flushSource, kVK_Shift, false);
    //     CGEventPost(kCGHIDEventTap, cmdUp);
    //     CGEventPost(kCGHIDEventTap, optUp);
    //     CGEventPost(kCGHIDEventTap, ctrlUp);
    //     CGEventPost(kCGHIDEventTap, shiftUp);
    //     CFRelease(cmdUp);
    //     CFRelease(optUp);
    //     CFRelease(ctrlUp);
    //     CFRelease(shiftUp);
    //     CFRelease(flushSource);
    // }

    // 然后等待所有修饰键真正松开
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

    CGEventSourceRef pasteSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    if (pasteSource) {
        // 按下 Command
        CGEventRef cmdDown = CGEventCreateKeyboardEvent(pasteSource, kVK_Command, true);
        CGEventPost(kCGHIDEventTap, cmdDown);
        CFRelease(cmdDown);
        usleep(20000);

        // 按下 V
        CGEventRef vDown = CGEventCreateKeyboardEvent(pasteSource, kVK_ANSI_V, true);
        CGEventPost(kCGHIDEventTap, vDown);
        CFRelease(vDown);
        usleep(20000);

        // 松开 V
        CGEventRef vUp = CGEventCreateKeyboardEvent(pasteSource, kVK_ANSI_V, false);
        CGEventPost(kCGHIDEventTap, vUp);
        CFRelease(vUp);
        usleep(20000);

        // 松开 Command
        CGEventRef cmdUp = CGEventCreateKeyboardEvent(pasteSource, kVK_Command, false);
        CGEventPost(kCGHIDEventTap, cmdUp);
        CFRelease(cmdUp);

        CFRelease(pasteSource);
    }
#endif
}

} // namespace ClipBridge
