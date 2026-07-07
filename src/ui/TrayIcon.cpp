#include "TrayIcon.h"
#include "SettingsDialog.h"
#include <QApplication>
#include <QMessageBox>

namespace ClipBridge {

TrayIcon::TrayIcon(const AppConfig &config, QObject *parent)
    : QSystemTrayIcon(parent), m_config(config)
{
    m_menu = new QMenu();

    m_settingsAction = new QAction(tr("设置"), this);
    m_aboutAction = new QAction(tr("关于"), this);
    m_quitAction = new QAction(tr("退出"), this);

    m_menu->addAction(m_settingsAction);
    m_menu->addSeparator();
    m_menu->addAction(m_aboutAction);
    m_menu->addSeparator();
    m_menu->addAction(m_quitAction);

    setContextMenu(m_menu);

    setToolTip(tr("ClipBridge - 剪贴板工具"));

    if (icon().isNull()) {
        setIcon(QIcon(":/resources/icon.svg"));
    }

    connect(m_settingsAction, &QAction::triggered, this, &TrayIcon::showSettings);
    connect(m_aboutAction, &QAction::triggered, this, &TrayIcon::showAbout);
    connect(m_quitAction, &QAction::triggered, this, &TrayIcon::quitRequested);
    connect(this, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {  // Left click
            showSettings();
        }
    });
}

void TrayIcon::showAbout()
{
    QMessageBox::about(nullptr,
        tr("关于 ClipBridge"),
        tr("<h3>ClipBridge</h3>"
           "<p>剪贴板工具 - 让终端中的图片粘贴更简单</p>"
           "<p>功能:</p>"
           "<ul>"
           "<li> 将剪贴板图片转为路径</li>"
           "<li> 去除文本换行符</li>"
           "</ul>"));
}

void TrayIcon::retranslateMenu()
{
    m_menu->hide();

    m_settingsAction->setText(tr("设置"));
    m_aboutAction->setText(tr("关于"));
    m_quitAction->setText(tr("退出"));
    setToolTip(tr("ClipBridge - 剪贴板工具"));
}

void TrayIcon::showSettings()
{
    SettingsDialog dialog(m_config);
    if (dialog.exec() == QDialog::Accepted) {
        m_config = dialog.config();
        emit configUpdated(m_config);
    }
}

} // namespace ClipBridge
