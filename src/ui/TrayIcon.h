#pragma once

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QPointer>

#include "core/AppConfig.h"

namespace ClipBridge {

class SettingsDialog;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit TrayIcon(const AppConfig &config, QObject *parent = nullptr);
    void retranslateMenu();

signals:
    void quitRequested();
    void configUpdated(const AppConfig &config);
    void monitorToggled(bool enabled);
    void autoStartToggled(bool enabled);
    void inputPanelRequested();

private slots:
    void showAbout();
    void showSettings();

private:
    AppConfig m_config;
    QMenu *m_menu;
    QAction *m_settingsAction;
    QAction *m_inputPanelAction;
    QAction *m_monitorAction;
    QAction *m_autoStartAction;
    QAction *m_aboutAction;
    QAction *m_quitAction;
    QPointer<SettingsDialog> m_settingsDialog;
};

} // namespace ClipBridge
