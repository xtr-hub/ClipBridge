/**
 * @file main.cpp
 * @brief 应用程序入口
 * @author Your Name
 * @date 2026
 */

#include <QApplication>
#include <QFileInfo>
#include <QDebug>
#include <QHash>

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

    QString configPath = QFileInfo(QCoreApplication::applicationDirPath()).filePath("config.json");
    AppConfig config = AppConfig::load(configPath);

    ActionManager manager(config);

    QHash<Hotkey*, AppConfig::Behavior> hotkeyBehaviors;
    for (const auto &binding : config.hotkeys) {
        Hotkey *hotkey = new Hotkey(binding.keySequence, true);

        if (hotkey->isRegistered()) {
            qDebug() << "Registered hotkey:" << binding.keySequence.toString()
                     << "for action:" << binding.action
                     << "(autoPaste:" << binding.behavior.autoPaste
                     << ", autoSubmit:" << binding.behavior.autoSubmit << ")";

            QObject::connect(hotkey, &Hotkey::activated, [&manager, binding]() {
                qDebug() << "Hotkey triggered, running action:" << binding.action;
                manager.run(binding.action, binding.behavior);
            });

            hotkeyBehaviors.insert(hotkey, binding.behavior);
        } else {
            qWarning() << "Failed to register hotkey:" << binding.keySequence.toString();
            delete hotkey;
        }
    }

    TrayIcon trayIcon(config);
    trayIcon.show();

    QObject::connect(&trayIcon, &TrayIcon::quitRequested, &app, &QApplication::quit);

    qDebug() << "ClipBridge started successfully!";

    return app.exec();
}
