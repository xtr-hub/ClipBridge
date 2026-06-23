#include "clipboardhelper.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDir>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_LINUX)
// TODO: Linux/X11 实现
#elif defined(Q_OS_MAC)
// TODO: macOS 实现
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
    // TODO: Linux/X11 实现 - XTestFakeKeyEvent
#elif defined(Q_OS_MAC)
    // TODO: macOS 实现 - CGEventPost
#endif
}

} // namespace ClipBridge
