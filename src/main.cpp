/**
 * @file main.cpp
 * @brief 应用程序入口
 * @author Your Name
 * @date 2026
 */

#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QHash>
#include <QPointer>
#include <QThread>

#include "core/AppConfig.h"
#include "core/ActionManager.h"
#include "ui/TrayIcon.h"
#include "utils/Hotkey.h"

using namespace ClipBridge;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("ClipBridge");
    QApplication::setApplicationVersion("2.0");
    QApplication::setOrganizationName("ClipBridge");

    QString configPath = QDir(QCoreApplication::applicationDirPath()).filePath("config.json");

    // 初始加载配置
    AppConfig config = AppConfig::load(configPath);
    ActionManager manager(config);

    // 热键管理
    QVector<QPointer<Hotkey>> hotkeys;
    auto registerHotkeys = [&](const AppConfig &newConfig) {
        // 清理旧热键
        for (auto hotkey : hotkeys) {
            if (hotkey) {
                delete hotkey;
            }
        }
        hotkeys.clear();

        // 注册新热键
        for (const auto &binding : newConfig.hotkeys) {
            Hotkey *hotkey = new Hotkey(binding.keySequence, true);

            if (hotkey->isRegistered()) {
                qDebug() << "Registered hotkey:" << binding.keySequence.toString()
                         << "for action:" << binding.action
                         << "(autoPaste:" << binding.behavior.autoPaste
                         << ", autoSubmit:" << binding.behavior.autoSubmit << ")";

                // 连接热键信号 - 这里我们使用值捕获，因为 binding 在循环结束后还会存在于 config 中
                QObject::connect(hotkey, &Hotkey::activated, [&manager, binding]() {
                    qDebug() << "Hotkey triggered, running action:" << binding.action;
                    manager.run(binding.action, binding.behavior);
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
        registerHotkeys(config);
        qDebug() << "Hot reload complete!";
    });

    QObject::connect(&trayIcon, &TrayIcon::quitRequested, &app, &QApplication::quit);

    qDebug() << "ClipBridge started successfully!";

    return app.exec();
}
