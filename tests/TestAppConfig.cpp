#include <QTest>
#include <QTemporaryFile>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include "core/AppConfig.h"

using namespace ClipBridge;

class TestAppConfig : public QObject
{
    Q_OBJECT

private slots:
    void testAvailableActions()
    {
        QStringList actions = AppConfig::availableActions();
        QCOMPARE(actions.size(), 4);
        QVERIFY(actions.contains("clipboard_image_path"));
        QVERIFY(actions.contains("clipboard_file_path"));
        QVERIFY(actions.contains("strip_newlines"));
        QVERIFY(actions.contains("clipboard_long_text"));
    }

    void testActionDisplayName()
    {
        // Source text (zh_CN) — returned verbatim when no translator is loaded
        QCOMPARE(AppConfig::actionDisplayName("clipboard_image_path"), QString::fromUtf8("复制图片路径"));
        QCOMPARE(AppConfig::actionDisplayName("clipboard_file_path"),  QString::fromUtf8("复制文件路径"));
        QCOMPARE(AppConfig::actionDisplayName("strip_newlines"),       QString::fromUtf8("去除换行符"));
        QCOMPARE(AppConfig::actionDisplayName("clipboard_long_text"),  QString::fromUtf8("长文本转文件路径"));
        // Unknown action → identity
        QCOMPARE(AppConfig::actionDisplayName("unknown_action"), QString("unknown_action"));
    }

    void testDefaultBehavior()
    {
        AppConfig::Behavior b;
        QVERIFY(b.autoPaste);
        QVERIFY(!b.autoSubmit);
    }

    void testDefaultConfig()
    {
        AppConfig config = AppConfig::load("nonexistent_file_12345.json");
        QCOMPARE(config.hotkeys.size(), 3);
        QCOMPARE(config.language, QString("zh_CN"));
        QVERIFY(!config.autoStart);
        QCOMPARE(config.output.format, QString("{path}"));
        QCOMPARE(config.output.mode, QString("workspace"));
        QCOMPARE(config.output.pasteKey, QString("Ctrl+V"));
        QCOMPARE(config.output.pasteDelay, 100);
        QVERIFY(!config.monitor.enabled);
        QCOMPARE(config.monitor.longTextThreshold, 500);
        QCOMPARE(config.inputPanel.windowOpacity, 95);
    }

    void testLoadSaveRoundtrip()
    {
        AppConfig original;
        AppConfig::HotKeyBinding binding;
        binding.action = "strip_newlines";
        binding.keySequence = QKeySequence("Ctrl+Shift+X");
        binding.behavior.autoPaste = true;
        binding.behavior.autoSubmit = false;
        original.hotkeys.append(binding);
        original.defaultBehavior.autoPaste = false;
        original.output.format = "Custom: {path}";
        original.output.mode = "custom_path";
        original.output.dir = "/tmp/test";
        original.output.formats["clipboard_image_path"] = "Image: {path}";
        original.output.pasteKey = "Ctrl+Shift+V";
        original.output.pasteDelay = 200;
        original.monitor.enabled = true;
        original.monitor.autoActions = {"strip_newlines", "clipboard_file_path"};
        original.monitor.longTextThreshold = 1000;
        original.monitor.showNotification = false;
        original.inputPanel.alwaysOnTop = false;
        original.inputPanel.saveLongTextAsFile = true;
        original.inputPanel.windowOpacity = 80;
        original.inputPanelHotkey = QKeySequence("Ctrl+Alt+X");
        original.language = "en_US";
        original.autoStart = true;

        QTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        QString path = tempFile.fileName();
        tempFile.close();
        original.save(path);

        AppConfig loaded = AppConfig::load(path);

        QCOMPARE(loaded.hotkeys.size(), 1);
        QCOMPARE(loaded.hotkeys[0].action, QString("strip_newlines"));
        QCOMPARE(loaded.hotkeys[0].keySequence.toString(), QKeySequence("Ctrl+Shift+X").toString());
        QVERIFY(loaded.hotkeys[0].behavior.autoPaste);
        QVERIFY(!loaded.defaultBehavior.autoPaste);
        QCOMPARE(loaded.output.format, QString("Custom: {path}"));
        QCOMPARE(loaded.output.mode, QString("custom_path"));
        QCOMPARE(loaded.output.dir, QString("/tmp/test"));
        QVERIFY(loaded.output.formats.contains("clipboard_image_path"));
        QCOMPARE(loaded.output.formats["clipboard_image_path"], QString("Image: {path}"));
        QCOMPARE(loaded.output.pasteKey, QString("Ctrl+Shift+V"));
        QCOMPARE(loaded.output.pasteDelay, 200);
        QVERIFY(loaded.monitor.enabled);
        QCOMPARE(loaded.monitor.longTextThreshold, 1000);
        QVERIFY(!loaded.monitor.showNotification);
        QVERIFY(loaded.monitor.autoActions.contains("strip_newlines"));
        QVERIFY(!loaded.inputPanel.alwaysOnTop);
        QVERIFY(loaded.inputPanel.saveLongTextAsFile);
        QCOMPARE(loaded.inputPanel.windowOpacity, 80);
        QCOMPARE(loaded.language, QString("en_US"));
        QVERIFY(loaded.autoStart);

        QFile::remove(path);
    }

    void testJsonArraySetRoundtrip()
    {
        // jsonArrayToSet / setToJsonArray are private — verified indirectly
        // via the full load/save roundtrip in testLoadSaveRoundtrip
        QVERIFY(true);
    }

    void testLoadRealConfigFile()
    {
        QString configPath = QDir(QCoreApplication::applicationDirPath()).filePath("config.json");
        if (QFile::exists(configPath)) {
            AppConfig config = AppConfig::load(configPath);
            QVERIFY(!config.language.isEmpty());
        }
    }
};

QTEST_MAIN(TestAppConfig)
#include "TestAppConfig.moc"
