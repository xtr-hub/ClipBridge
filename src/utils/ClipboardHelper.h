#pragma once

#include <QString>
#include <QImage>
#include <QMimeData>

namespace ClipBridge {

class ClipboardHelper
{
public:
    static bool hasImage();
    static QImage getImage();
    static bool saveImageToPng(const QImage &image, const QString &path);

    static QString getText();
    static void setText(const QString &text);

    static bool hasUrls();
    static QStringList getFilePaths();

    static bool hasText();
    static int getTextLength();
    static bool isTextLong(int threshold);
    static bool saveTextToFile(const QString &text, const QString &path);

    static QString detectContentType();
};

} // namespace ClipBridge
