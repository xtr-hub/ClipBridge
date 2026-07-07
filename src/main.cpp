/**
 * @file main.cpp
 * @brief 应用程序入口
 * @author Your Name
 * @date 2026
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QHash>
#include <QPointer>
#include <QThread>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>

#include "core/AppConfig.h"
#include "core/ActionManager.h"
#include "ui/TrayIcon.h"
#include "utils/Hotkey.h"

using namespace ClipBridge;

int main(int argc, char *argv[])
{
#ifdef _WIN32
    // 隐藏控制台窗口
    FreeConsole();
#endif

    // 注册自定义类型，用于信号槽
    qRegisterMetaType<AppConfig::Behavior>("AppConfig::Behavior");

    QApplication app(argc, argv);

    // macOS: 防止没有窗口时应用自动退出
    QApplication::setQuitOnLastWindowClosed(false);

    QApplication::setApplicationName("ClipBridge");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("ClipBridge");
    QApplication::setWindowIcon(QIcon(":/resources/icon.svg"));

    // 启用深色模式自适应
    app.setStyle("Fusion");

    QString configPath = QDir(QCoreApplication::applicationDirPath()).filePath("config.json");

    // 初始加载配置
    AppConfig config = AppConfig::load(configPath);

    // 安装翻译器（中文为源语言，直接使用 source 文本）
    QTranslator appTranslator;
    QTranslator qtTranslator;

    QString language = config.language.isEmpty() ? QLocale::system().name() : config.language;

    if (language != "zh_CN") {
        QString qmPath = QString(":/resources/translations/ClipBridge_%1.qm").arg(language);
        if (appTranslator.load(qmPath)) {
            app.installTranslator(&appTranslator);
            qDebug() << "Loaded app translation:" << language;
        } else {
            qDebug() << "No app translation found for:" << language;
        }
    }

    QString qtQmPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath) + QString("/qt_%1.qm").arg(language);
    if (qtTranslator.load(qtQmPath)) {
        app.installTranslator(&qtTranslator);
    }

    ActionManager manager(config);

    // 热键管理
    QVector<QPointer<Hotkey>> hotkeys;

    auto registerHotkeys = [&](const AppConfig &newConfig) {
        // 只 unregister 旧热键，不 delete！防止崩溃！
        for (auto hotkey : hotkeys) {
            if (hotkey) {
                hotkey->unregisterHotkey();
            }
        }
        hotkeys.clear();

        // 注册新热键
        for (const auto &binding : newConfig.hotkeys) {
            Hotkey *hotkey = new Hotkey(binding.action, binding.keySequence, binding.behavior, true);

            if (hotkey->isRegistered()) {
                qDebug() << "Registered hotkey:" << binding.keySequence.toString()
                         << "for action:" << binding.action
                         << "(autoPaste:" << binding.behavior.autoPaste
                         << ", autoSubmit:" << binding.behavior.autoSubmit << ")";

                // 连接热键信号
                QObject::connect(hotkey, &Hotkey::activated, [&manager](const QString &action, const AppConfig::Behavior &behavior) {
                    qDebug() << "Hotkey triggered, running action:" << action;
                    manager.run(action, behavior);
                });

                hotkeys.append(hotkey);
            } else {
                qWarning() << "Failed to register hotkey:" << binding.keySequence.toString();
                delete hotkey;
            }
        }
    };

    // 初始注册热键
    registerHotkeys(config);

    TrayIcon trayIcon(config);
    trayIcon.show();

    // 连接配置更新信号
    QObject::connect(&trayIcon, &TrayIcon::configUpdated, [&](const AppConfig &newConfig) {
        qDebug() << "Config updated, reloading...";
        config = newConfig;
        manager.updateConfig(config);

        // 热更新语言
        QString newLanguage = config.language.isEmpty() ? QLocale::system().name() : config.language;
        if (newLanguage != language) {
            language = newLanguage;
            app.removeTranslator(&appTranslator);
            app.removeTranslator(&qtTranslator);

            if (language != "zh_CN") {
                QString qmPath = QString(":/resources/translations/ClipBridge_%1.qm").arg(language);
                if (appTranslator.load(qmPath)) {
                    app.installTranslator(&appTranslator);
                    qDebug() << "Reloaded app translation:" << language;
                } else {
                    qDebug() << "No app translation found for:" << language;
                }
            }

            QString qtQmPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath) + QString("/qt_%1.qm").arg(language);
            if (qtTranslator.load(qtQmPath)) {
                app.installTranslator(&qtTranslator);
            }

            trayIcon.retranslateMenu();
        }

        registerHotkeys(config);
        qDebug() << "Hot reload complete!";
    });

    QObject::connect(&trayIcon, &TrayIcon::quitRequested, &app, &QApplication::quit);

    qDebug() << "ClipBridge started successfully!";

    return app.exec();
}
