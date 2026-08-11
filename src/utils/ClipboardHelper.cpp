#include "ClipboardHelper.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace ClipBridge {

bool ClipboardHelper::hasImage()
{
    const QMimeData *d = QApplication::clipboard()->mimeData();
    return d && d->hasImage();
}

QImage ClipboardHelper::getImage()
{
    const QMimeData *d = QApplication::clipboard()->mimeData();
    if (d && d->hasImage())
        return qvariant_cast<QImage>(d->imageData());
    return QImage();
}

bool ClipboardHelper::saveImageToPng(const QImage &image, const QString &path)
{
    if (image.isNull()) return false;
    QFileInfo fi(path);
    QDir dir = fi.absoluteDir();
    if (!dir.exists()) dir.mkpath(".");
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

bool ClipboardHelper::hasUrls()
{
    const QMimeData *d = QApplication::clipboard()->mimeData();
    return d && d->hasUrls();
}

QStringList ClipboardHelper::getFilePaths()
{
    const QMimeData *d = QApplication::clipboard()->mimeData();
    if (!d || !d->hasUrls()) return {};
    QStringList paths;
    for (const QUrl &url : d->urls()) {
        if (url.isLocalFile()) {
            QString p = url.toLocalFile();
            if (!p.isEmpty()) paths.append(p);
        }
    }
    return paths;
}

bool ClipboardHelper::hasText()
{
    const QMimeData *d = QApplication::clipboard()->mimeData();
    return d && d->hasText();
}

int ClipboardHelper::getTextLength()
{
    return getText().length();
}

bool ClipboardHelper::isTextLong(int threshold)
{
    return threshold > 0 && getTextLength() > threshold;
}

bool ClipboardHelper::saveTextToFile(const QString &text, const QString &path)
{
    QFileInfo fi(path);
    QDir dir = fi.absoluteDir();
    if (!dir.exists()) dir.mkpath(".");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#endif
    stream << text;
    return true;
}

QString ClipboardHelper::detectContentType()
{
    const QMimeData *m = QApplication::clipboard()->mimeData();
    if (!m) return "none";
    if (m->hasImage()) return "image";
    if (m->hasUrls())  return "files";
    if (m->hasText())  return "text";
    return "unknown";
}

} // namespace ClipBridge
