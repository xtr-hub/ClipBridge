#include "TrayIcon.h"
#include "SettingsDialog.h"
#include "core/AutoStart.h"
#include <QApplication>
#include <QMessageBox>

namespace ClipBridge {

TrayIcon::TrayIcon(const AppConfig &config, QObject *parent)
    : QSystemTrayIcon(parent), m_config(config)
{
    m_menu = new QMenu();

    m_settingsAction = new QAction(tr("设置"), this);
    m_inputPanelAction = new QAction(tr("输入面板"), this);

    m_monitorAction = new QAction(tr("监听模式"), this);
    m_monitorAction->setCheckable(true);
    m_monitorAction->setChecked(config.monitor.enabled);

    m_autoStartAction = new QAction(tr("开机自启"), this);
    m_autoStartAction->setCheckable(true);
    m_autoStartAction->setChecked(config.autoStart);

    m_aboutAction = new QAction(tr("关于"), this);
    m_quitAction = new QAction(tr("退出"), this);

    m_menu->addAction(m_settingsAction);
    m_menu->addAction(m_inputPanelAction);
    m_menu->addSeparator();
    m_menu->addAction(m_monitorAction);
    m_menu->addAction(m_autoStartAction);
    m_menu->addSeparator();
    m_menu->addAction(m_aboutAction);
    m_menu->addSeparator();
    m_menu->addAction(m_quitAction);

    setContextMenu(m_menu);
    setToolTip(tr("ClipBridge - 剪贴板工具"));

    if (icon().isNull())
        setIcon(QIcon(":/resources/icon.png"));

    connect(m_settingsAction, &QAction::triggered, this, &TrayIcon::showSettings);
    connect(m_aboutAction, &QAction::triggered, this, &TrayIcon::showAbout);
    connect(m_quitAction, &QAction::triggered, this, &TrayIcon::quitRequested);

    connect(m_inputPanelAction, &QAction::triggered, this, &TrayIcon::inputPanelRequested);

    connect(m_monitorAction, &QAction::toggled, this, [this](bool checked) {
        m_config.monitor.enabled = checked;
        emit monitorToggled(checked);
    });

    connect(m_autoStartAction, &QAction::toggled, this, [this](bool checked) {
        m_config.autoStart = checked;
        AutoStart::setEnabled(checked);
        emit autoStartToggled(checked);
    });

    connect(this, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger)
            showSettings();
    });
}

void TrayIcon::showAbout()
{
    QMessageBox::about(nullptr,
        tr("关于 ClipBridge"),
        tr("<h3>ClipBridge</h3>"
           "<p>剪贴板工具 - 让终端中的剪贴板交互更顺畅</p>"
           "<p>功能:</p>"
           "<ul>"
           "<li>图片转路径</li>"
           "<li>文件路径复制</li>"
           "<li>去除换行符</li>"
           "<li>长文本转文件</li>"
           "<li>输入面板</li>"
           "<li>监听模式</li>"
           "</ul>"));
}

void TrayIcon::retranslateMenu()
{
    m_menu->hide();
    m_settingsAction->setText(tr("设置"));
    m_inputPanelAction->setText(tr("输入面板"));
    m_monitorAction->setText(tr("监听模式"));
    m_autoStartAction->setText(tr("开机自启"));
    m_aboutAction->setText(tr("关于"));
    m_quitAction->setText(tr("退出"));
    setToolTip(tr("ClipBridge - 剪贴板工具"));
}

void TrayIcon::showSettings()
{
    if (m_settingsDialog) {
        m_settingsDialog->raise();
        m_settingsDialog->activateWindow();
        return;
    }

    SettingsDialog *dialog = new SettingsDialog(m_config);
    m_settingsDialog = dialog;

    connect(dialog, &QDialog::finished, [this, dialog](int result) {
        if (result == QDialog::Accepted) {
            m_config = dialog->config();
            m_monitorAction->setChecked(m_config.monitor.enabled);
            m_autoStartAction->setChecked(m_config.autoStart);
            emit configUpdated(m_config);
        }
        m_settingsDialog = nullptr;
        dialog->deleteLater();
    });

    dialog->show();
}

} // namespace ClipBridge
