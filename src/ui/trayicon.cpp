#include "trayicon.h"
#include <QApplication>
#include <QMessageBox>

namespace ClipBridge {

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
{
    // 创建菜单
    m_menu = new QMenu();

    m_aboutAction = new QAction(tr("关于"), this);
    connect(m_aboutAction, &QAction::triggered, this, &TrayIcon::showAbout);

    m_quitAction = new QAction(tr("退出"), this);
    connect(m_quitAction, &QAction::triggered, this, &TrayIcon::quitRequested);

    m_menu->addAction(m_aboutAction);
    m_menu->addSeparator();
    m_menu->addAction(m_quitAction);

    setContextMenu(m_menu);

    // 设置图标（使用默认应用图标）
    setToolTip(tr("ClipBridge - 剪贴板工具"));

    // 如果没有设置图标，使用系统默认
    if (icon().isNull()) {
        setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    }
}

void TrayIcon::showAbout()
{
    QMessageBox::about(nullptr,
        tr("关于 ClipBridge"),
        tr("<h3>ClipBridge</h3>"
           "<p>剪贴板工具 - 让终端中的图片粘贴更简单</p>"
           "<p>快捷键:</p>"
           "<ul>"
           "<li>Ctrl+Alt+I: 将剪贴板图片转为路径</li>"
           "<li>Ctrl+Alt+J: 去除文本换行符</li>"
           "</ul>"));
}

} // namespace ClipBridge
