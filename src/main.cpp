#ifdef _WIN32
#include <windows.h>
#endif

#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QHash>
#include <QPointer>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>

#include "core/AppConfig.h"
#include "core/ActionManager.h"
#include "core/MonitorService.h"
#include "core/AutoStart.h"
#include "ui/TrayIcon.h"
#include "ui/InputPanel.h"
#include "utils/Hotkey.h"

using namespace ClipBridge;

static QString qtTranslationsPath()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    return QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    FreeConsole();
#endif

    qRegisterMetaType<AppConfig::Behavior>("AppConfig::Behavior");

    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
    QApplication::setApplicationName("ClipBridge");
    QApplication::setApplicationVersion("1.1.0");
    QApplication::setOrganizationName("ClipBridge");
    QApplication::setWindowIcon(QIcon(":/resources/icon.png"));
    app.setStyle("Fusion");

    QString configPath = QDir(QCoreApplication::applicationDirPath()).filePath("config.json");
    AppConfig config = AppConfig::load(configPath);

    if (config.autoStart)
        AutoStart::setEnabled(true);

    // Translations
    QTranslator appTranslator, qtTranslator;
    QString language = config.language.isEmpty() ? QLocale::system().name() : config.language;
    if (language != "zh_CN") {
        QString qmPath = QString(":/resources/translations/ClipBridge_%1.qm").arg(language);
        if (appTranslator.load(qmPath))
            app.installTranslator(&appTranslator);
    }
    QString qtQm = qtTranslationsPath() + QString("/qt_%1.qm").arg(language);
    if (qtTranslator.load(qtQm))
        app.installTranslator(&qtTranslator);

    // Core
    ActionManager manager(config);
    MonitorService monitor(&manager);
    monitor.updateConfig(config.monitor, config.output.pasteKey);
    monitor.setEnabled(config.monitor.enabled);

    InputPanel inputPanel(&manager);
    inputPanel.updateConfig(config);

    // Input panel ↔ monitor exclusion (ref-counted — nesting-safe)
    QObject::connect(&inputPanel, &InputPanel::panelOpened, [&]() {
        monitor.enterIgnoreScope();
    });
    QObject::connect(&inputPanel, &InputPanel::panelClosed, [&]() {
        monitor.leaveIgnoreScope();
    });

    // Hotkeys
    QVector<QPointer<Hotkey>> hotkeys;
    auto registerHotkeys = [&](const AppConfig &cfg) {
        for (auto hk : hotkeys) { if (hk) hk->unregisterHotkey(); }
        hotkeys.clear();

        for (const auto &b : cfg.hotkeys) {
            Hotkey *hk = new Hotkey(b.action, b.keySequence, b.behavior, true);
            if (hk->isRegistered()) {
                QObject::connect(hk, &Hotkey::activated,
                    [&manager](const QString &a, const AppConfig::Behavior &bhv) {
                        manager.run(a, bhv);
                    });
                hotkeys.append(hk);
            } else {
                delete hk;
            }
        }

        if (!cfg.inputPanelHotkey.isEmpty()) {
            Hotkey *ph = new Hotkey("__input_panel__", cfg.inputPanelHotkey,
                                    AppConfig::Behavior(), true);
            if (ph->isRegistered()) {
                QObject::connect(ph, &Hotkey::activated,
                    [&inputPanel](const QString &, const AppConfig::Behavior &) {
                        inputPanel.showAndRefresh();
                    });
                hotkeys.append(ph);
            } else {
                delete ph;
            }
        }
    };
    registerHotkeys(config);

    // Tray
    TrayIcon trayIcon(config);
    trayIcon.show();

    QObject::connect(&trayIcon, &TrayIcon::configUpdated, [&](const AppConfig &cfg) {
        config = cfg;
        manager.updateConfig(config);
        monitor.updateConfig(config.monitor, config.output.pasteKey);
        monitor.setEnabled(config.monitor.enabled);
        inputPanel.updateConfig(config);

        QString nl = config.language.isEmpty() ? QLocale::system().name() : config.language;
        if (nl != language) {
            language = nl;
            app.removeTranslator(&appTranslator);
            app.removeTranslator(&qtTranslator);
            if (language != "zh_CN") {
                QString qp = QString(":/resources/translations/ClipBridge_%1.qm").arg(language);
                if (appTranslator.load(qp)) app.installTranslator(&appTranslator);
            }
            QString qq = qtTranslationsPath() + QString("/qt_%1.qm").arg(language);
            if (qtTranslator.load(qq)) app.installTranslator(&qtTranslator);
            trayIcon.retranslateMenu();
        }
        registerHotkeys(config);
    });

    QObject::connect(&trayIcon, &TrayIcon::monitorToggled, [&](bool on) {
        monitor.setEnabled(on);
    });
    QObject::connect(&trayIcon, &TrayIcon::autoStartToggled, [](bool on) {
        AutoStart::setEnabled(on);
    });
    QObject::connect(&trayIcon, &TrayIcon::inputPanelRequested, [&]() {
        inputPanel.showAndRefresh();
    });
    QObject::connect(&trayIcon, &TrayIcon::quitRequested, &app, &QApplication::quit);

    return app.exec();
}
