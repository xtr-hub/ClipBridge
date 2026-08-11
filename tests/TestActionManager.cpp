#include <QTest>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include "core/ActionManager.h"
#include "core/AppConfig.h"
#include "utils/ClipboardHelper.h"

using namespace ClipBridge;

class TestActionManager : public QObject
{
    Q_OBJECT

private:
    ActionManager *m_actionManager = nullptr;
    QTemporaryDir m_tempDir;

private slots:
    void initTestCase()
    {
        AppConfig config;
        config.output.mode = "custom_path";
        config.output.dir = m_tempDir.path();
        m_actionManager = new ActionManager(config);
    }

    void cleanupTestCase()
    {
        delete m_actionManager;
        m_actionManager = nullptr;
    }

    void cleanup()
    {
        QApplication::clipboard()->clear();
    }

    void testCanHandleAction_data()
    {
        QTest::addColumn<QString>("action");
        QTest::addColumn<QString>("contentType");
        QTest::addColumn<bool>("expected");

        QTest::newRow("image->image")   << "clipboard_image_path" << "image" << true;
        QTest::newRow("image->text")     << "clipboard_image_path" << "text"  << false;
        QTest::newRow("image->files")    << "clipboard_image_path" << "files" << false;
        QTest::newRow("files->files")    << "clipboard_file_path"  << "files" << true;
        QTest::newRow("files->image")    << "clipboard_file_path"  << "image" << false;
        QTest::newRow("strip->text")     << "strip_newlines"       << "text"  << true;
        QTest::newRow("strip->image")    << "strip_newlines"       << "image" << false;
        QTest::newRow("long->text")      << "clipboard_long_text"  << "text"  << true;
        QTest::newRow("long->files")     << "clipboard_long_text"  << "files" << false;
        QTest::newRow("unknown")        << "nonexistent"          << "text"  << false;
    }

    void testCanHandleAction()
    {
        QFETCH(QString, action);
        QFETCH(QString, contentType);
        QFETCH(bool, expected);
        QCOMPARE(ActionManager::canHandleAction(action, contentType), expected);
    }

    void testProcessStripNewlines()
    {
        QApplication::clipboard()->setText("hello\nworld\r\ntest\n");
        QString result = m_actionManager->process("strip_newlines");
        QCOMPARE(result, QString("helloworldtest"));
    }

    void testProcessStripNewlinesEmpty()
    {
        QApplication::clipboard()->clear();
        QString result = m_actionManager->process("strip_newlines");
        QVERIFY(result.isEmpty());
    }

    void testProcessClipboardFilePath()
    {
        // Use temp path to get platform-correct path
        QString testFile1 = QDir::tempPath() + "/test_clipbridge_a.txt";
        QString testFile2 = QDir::tempPath() + "/other_clipbridge_b.pdf";

        QMimeData *mime = new QMimeData();
        QList<QUrl> urls;
        urls.append(QUrl::fromLocalFile(testFile1));
        urls.append(QUrl::fromLocalFile(testFile2));
        mime->setUrls(urls);
        QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);

        QString result = m_actionManager->process("clipboard_file_path");
        QVERIFY(result.contains("test_clipbridge_a.txt"));
        QVERIFY(result.contains("other_clipbridge_b.pdf"));
    }

    void testProcessClipboardFilePathEmpty()
    {
        QApplication::clipboard()->clear();
        QString result = m_actionManager->process("clipboard_file_path");
        QVERIFY(result.isEmpty());
    }

    void testProcessClipboardImagePath()
    {
        QImage img(16, 16, QImage::Format_ARGB32);
        img.fill(Qt::red);
        QApplication::clipboard()->setImage(img);

        QString result = m_actionManager->process("clipboard_image_path");
        QVERIFY(!result.isEmpty());
        QVERIFY(QFile::exists(result));
        QVERIFY(result.endsWith(".png"));

        QFile::remove(result);
    }

    void testProcessClipboardImagePathNoImage()
    {
        QApplication::clipboard()->clear();
        QString result = m_actionManager->process("clipboard_image_path");
        QVERIFY(result.isEmpty());
    }

    void testProcessLongTextShort()
    {
        QApplication::clipboard()->setText("short message");
        QString result = m_actionManager->process("clipboard_long_text");
        QCOMPARE(result, QString("short message"));
    }

    void testProcessLongTextOverThreshold()
    {
        QString longText(600, 'A');
        QApplication::clipboard()->setText(longText);

        QString result = m_actionManager->process("clipboard_long_text");
        QVERIFY(!result.isEmpty());
        QVERIFY(result.endsWith(".txt"));
        QVERIFY(QFile::exists(result));

        QFile f(result);
        QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
        QString content = QString::fromUtf8(f.readAll());
        QCOMPARE(content, longText);

        f.close();
        QFile::remove(result);
    }

    void testProcessLongTextEmpty()
    {
        QApplication::clipboard()->clear();
        QString result = m_actionManager->process("clipboard_long_text");
        QVERIFY(result.isEmpty());
    }

    void testProcessUnknownAction()
    {
        QString result = m_actionManager->process("nonexistent_action");
        QVERIFY(result.isEmpty());
    }

    void testLongTextThreshold()
    {
        QCOMPARE(m_actionManager->longTextThreshold(), 500);
    }

    void testUpdateConfig()
    {
        AppConfig newConfig;
        newConfig.monitor.longTextThreshold = 2000;
        m_actionManager->updateConfig(newConfig);
        QCOMPARE(m_actionManager->longTextThreshold(), 2000);

        AppConfig original;
        original.output.mode = "custom_path";
        original.output.dir = m_tempDir.path();
        m_actionManager->updateConfig(original);
    }

    void testProcessWithCustomFormat()
    {
        AppConfig config;
        config.output.format = "Check this: {path}";
        config.output.formats["clipboard_image_path"] = "Image here: {path}";
        config.output.mode = "custom_path";
        config.output.dir = m_tempDir.path();
        m_actionManager->updateConfig(config);

        QString testPath = QDir::tempPath() + "/clipbridge_format_test.txt";
        QMimeData *mime = new QMimeData();
        QList<QUrl> urls;
        urls.append(QUrl::fromLocalFile(testPath));
        mime->setUrls(urls);
        QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);

        QString result = m_actionManager->process("clipboard_file_path");
        QCOMPARE(result, QString("Check this: ") + testPath);

        // Restore default config
        AppConfig original;
        original.output.mode = "custom_path";
        original.output.dir = m_tempDir.path();
        m_actionManager->updateConfig(original);
    }
};

QTEST_MAIN(TestActionManager)
#include "TestActionManager.moc"
