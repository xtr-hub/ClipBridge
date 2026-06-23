/**
 * @file main.cpp
 * @brief 应用程序入口
 * @author Your Name
 * @date 2026
 */

#include <QApplication>
#include <QFileInfo>
#include <QDebug>

#include "core/appconfig.h"
#include "core/actionmanager.h"
#include "ui/trayicon.h"
#include "utils/hotkey.h"

using namespace ClipBridge;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用信息
    QApplication::setApplicationName("ClipBridge");
    QApplication::setApplicationVersion("2.0");
    QApplication::setOrganizationName("ClipBridge");

    // 加载配置
    QString configPath = QFileInfo(QCoreApplication::applicationDirPath())
                             .filePath("config.json");
    AppConfig config = AppConfig::load(configPath);

    // 创建动作管理器
    ActionManager manager(config);

    // 注册热键
    QList<Hotkey *> hotkeys;
    for (const auto &binding : config.hotkeys) {
        Hotkey *hotkey = new Hotkey(binding.keySequence, true);

        if (hotkey->isRegistered()) {
            qDebug() << "Registered hotkey:" << binding.keySequence.toString()
                     << "for action:" << binding.action;

            QObject::connect(hotkey, &Hotkey::activated, [&manager, binding]() {
                qDebug() << "Hotkey triggered, running action:" << binding.action;
                manager.run(binding.action);
            });

            hotkeys.append(hotkey);
        } else {
            qWarning() << "Failed to register hotkey:" << binding.keySequence.toString();
            delete hotkey;
        }
    }

    // 创建托盘图标
    TrayIcon trayIcon;
    trayIcon.show();

    QObject::connect(&trayIcon, &TrayIcon::quitRequested, &app, &QApplication::quit);

    qDebug() << "ClipBridge started successfully";

    return app.exec();
}
