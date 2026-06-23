#include "TrayIcon.h"
#include "SettingsDialog.h"
#include <QApplication>
#include <QMessageBox>

namespace ClipBridge {

TrayIcon::TrayIcon(const AppConfig &config, QObject *parent)
    : QSystemTrayIcon(parent), m_config(config)
{
    m_menu = new QMenu();

    m_settingsAction = new QAction("设置", this);
    m_aboutAction = new QAction("关于", this);
    m_quitAction = new QAction("退出", this);

    m_menu->addAction(m_settingsAction);
    m_menu->addSeparator();
    m_menu->addAction(m_aboutAction);
    m_menu->addSeparator();
    m_menu->addAction(m_quitAction);

    setContextMenu(m_menu);

    setToolTip("ClipBridge - 剪贴板工具");

    if (icon().isNull()) {
        setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    }

    connect(m_settingsAction, &QAction::triggered, this, &TrayIcon::showSettings);
    connect(m_aboutAction, &QAction::triggered, this, &TrayIcon::showAbout);
    connect(m_quitAction, &QAction::triggered, this, &TrayIcon::quitRequested);
}

void TrayIcon::showAbout()
{
    QMessageBox::about(nullptr,
        "关于 ClipBridge",
        "<h3>ClipBridge</h3>"
        "<p>剪贴板工具 - 让终端中的图片粘贴更简单</p>"
        "<p>快捷键:</p>"
        "<ul>"
        "<li>Ctrl+Alt+I: 将剪贴板图片转为路径</li>"
        "<li>Ctrl+Alt+J: 去除文本换行符</li>"
        "</ul>");
}

void TrayIcon::showSettings()
{
    SettingsDialog dialog(m_config);
    if (dialog.exec() == QDialog::Accepted) {
        m_config = dialog.config();
    }
}

} // namespace ClipBridge
