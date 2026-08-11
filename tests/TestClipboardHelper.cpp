#include <QTest>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include "utils/ClipboardHelper.h"

using namespace ClipBridge;

class TestClipboardHelper : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;

private slots:
    void cleanup()
    {
        QApplication::clipboard()->clear();
    }

    void testHasImageInitially()
    {
        QVERIFY(!ClipboardHelper::hasImage());
    }

    void testGetImageEmpty()
    {
        QImage img = ClipboardHelper::getImage();
        QVERIFY(img.isNull());
    }

    void testSetAndGetText()
    {
        ClipboardHelper::setText("Hello World");
        QCOMPARE(ClipboardHelper::getText(), QString("Hello World"));
    }

    void testHasText()
    {
        QVERIFY(!ClipboardHelper::hasText());
        ClipboardHelper::setText("test");
        QVERIFY(ClipboardHelper::hasText());
    }

    void testGetTextLength()
    {
        ClipboardHelper::setText("12345");
        QCOMPARE(ClipboardHelper::getTextLength(), 5);
    }

    void testIsTextLong()
    {
        ClipboardHelper::setText(QString(100, 'x'));
        QVERIFY(!ClipboardHelper::isTextLong(500));
        QVERIFY(ClipboardHelper::isTextLong(50));
        QVERIFY(!ClipboardHelper::isTextLong(0));
    }

    void testHasUrlsInitially()
    {
        QVERIFY(!ClipboardHelper::hasUrls());
    }

    void testGetFilePathsEmpty()
    {
        QStringList paths = ClipboardHelper::getFilePaths();
        QVERIFY(paths.isEmpty());
    }

    void testGetFilePaths()
    {
        QMimeData *mime = new QMimeData();
        QList<QUrl> urls;
        urls.append(QUrl::fromLocalFile("/home/user/file.txt"));
        urls.append(QUrl::fromLocalFile("/tmp/image.png"));
        urls.append(QUrl("https://example.com"));
        mime->setUrls(urls);
        QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);

        QStringList paths = ClipboardHelper::getFilePaths();
        QCOMPARE(paths.size(), 2);
        QVERIFY(paths.contains("/home/user/file.txt"));
        QVERIFY(paths.contains("/tmp/image.png"));
        QVERIFY(!paths.contains("https://example.com"));
    }

    void testDetectContentType()
    {
        QApplication::clipboard()->clear();
        QString empty = ClipboardHelper::detectContentType();
        QStringList valid = {"none", "unknown"};
        QVERIFY2(valid.contains(empty),
                 qPrintable(QString("Expected none/unknown, got: %1").arg(empty)));

        ClipboardHelper::setText("hello");
        QCOMPARE(ClipboardHelper::detectContentType(), QString("text"));

        QImage img(10, 10, QImage::Format_ARGB32);
        img.fill(Qt::blue);
        QApplication::clipboard()->setImage(img);
        QCOMPARE(ClipboardHelper::detectContentType(), QString("image"));

        QMimeData *mime = new QMimeData();
        mime->setUrls({QUrl::fromLocalFile("/tmp/f")});
        QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
        QCOMPARE(ClipboardHelper::detectContentType(), QString("files"));
    }

    void testSaveTextToFile()
    {
        QString filePath = m_tempDir.path() + "/test_output.txt";
        bool ok = ClipboardHelper::saveTextToFile("Hello\nClipBridge", filePath);
        QVERIFY(ok);
        QVERIFY(QFile::exists(filePath));

        QFile f(filePath);
        QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
        QString content = QString::fromUtf8(f.readAll());
        QCOMPARE(content, QString("Hello\nClipBridge"));
    }

    void testSaveTextToFileNestedDir()
    {
        QString filePath = m_tempDir.path() + "/sub/dir/deep/output.txt";
        bool ok = ClipboardHelper::saveTextToFile("nested", filePath);
        QVERIFY(ok);
        QVERIFY(QFile::exists(filePath));
    }

    void testSaveImageToPng()
    {
        QImage img(32, 32, QImage::Format_ARGB32);
        img.fill(Qt::green);
        QString filePath = m_tempDir.path() + "/test_image.png";

        bool ok = ClipboardHelper::saveImageToPng(img, filePath);
        QVERIFY(ok);
        QVERIFY(QFile::exists(filePath));

        QImage loaded(filePath);
        QVERIFY(!loaded.isNull());
        QCOMPARE(loaded.size(), QSize(32, 32));
    }

    void testSaveImageToPngNull()
    {
        QImage nullImg;
        bool ok = ClipboardHelper::saveImageToPng(nullImg, "/tmp/should_not_exist.png");
        QVERIFY(!ok);
    }
};

QTEST_MAIN(TestClipboardHelper)
#include "TestClipboardHelper.moc"
