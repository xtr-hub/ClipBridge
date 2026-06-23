#include "clipboardhelper.h"
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
    // 模拟 Ctrl+V
    keybd_event(VK_CONTROL, 0, 0, 0);
    keybd_event('V', 0, 0, 0);
    keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
#elif defined(Q_OS_LINUX)
    // 模拟 Ctrl+V
    Display *dpy = XOpenDisplay(nullptr);
    if (dpy) {
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), True, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_v), True, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_v), False, 0);
        XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), False, 0);
        XFlush(dpy);
        XCloseDisplay(dpy);
    }
#elif defined(Q_OS_MAC)
    // 模拟 Cmd+V
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    if (source) {
        CGEventRef cmdDown = CGEventCreateKeyboardEvent(source, kVK_Command, true);
        CGEventRef vDown = CGEventCreateKeyboardEvent(source, kVK_ANSI_V, true);
        CGEventRef vUp = CGEventCreateKeyboardEvent(source, kVK_ANSI_V, false);
        CGEventRef cmdUp = CGEventCreateKeyboardEvent(source, kVK_Command, false);

        CGEventPost(kCGHIDEventTap, cmdDown);
        CGEventPost(kCGHIDEventTap, vDown);
        CGEventPost(kCGHIDEventTap, vUp);
        CGEventPost(kCGHIDEventTap, cmdUp);

        CFRelease(cmdDown);
        CFRelease(vDown);
        CFRelease(vUp);
        CFRelease(cmdUp);
        CFRelease(source);
    }
#endif
}

} // namespace ClipBridge
