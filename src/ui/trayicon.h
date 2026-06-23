#pragma once

/**
 * @file trayicon.h
 * @brief 系统托盘图标
 * @author Your Name
 * @date 2026
 */

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

#include "core/appconfig.h"

namespace ClipBridge {

/**
 * @brief 系统托盘图标类
 */
class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param config 应用配置
     * @param parent 父对象
     */
    explicit TrayIcon(const AppConfig &config, QObject *parent = nullptr);

signals:
    /**
     * @brief 请求退出应用
     */
    void quitRequested();

private slots:
    /**
     * @brief 显示关于对话框
     */
    void showAbout();

    /**
     * @brief 显示设置对话框
     */
    void showSettings();

private:
    AppConfig m_config;
    QMenu *m_menu;
    QAction *m_settingsAction;
    QAction *m_aboutAction;
    QAction *m_quitAction;
};

} // namespace ClipBridge
