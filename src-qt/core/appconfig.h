#pragma once

/**
 * @file appconfig.h
 * @brief 应用程序配置管理
 * @author Your Name
 * @date 2026
 */

#include <QString>
#include <QVector>
#include <QKeySequence>

namespace ClipBridge {

/**
 * @brief 应用程序配置结构
 */
struct AppConfig
{
    /**
     * @brief 热键绑定结构
     */
    struct HotKeyBinding
    {
        QString action;             ///< 动作名称
        QKeySequence keySequence;   ///< 按键序列
    };

    /**
     * @brief 输出配置
     */
    struct Output
    {
        QString format = "{path}";  ///< 输出格式模板
        QString mode = "workspace"; ///< 路径模式: workspace/custom_path
        QString dir;                ///< 自定义目录
    };

    /**
     * @brief 行为配置
     */
    struct Behavior
    {
        bool autoPaste = true;      ///< 是否自动粘贴
        bool autoSubmit = false;    ///< 是否自动提交
    };

    QVector<HotKeyBinding> hotkeys;  ///< 热键列表
    Behavior behavior;               ///< 行为配置
    Output output;                   ///< 输出配置

    /**
     * @brief 从文件加载配置
     * @param path 配置文件路径
     * @return 配置对象
     */
    static AppConfig load(const QString &path);

    /**
     * @brief 保存配置到文件
     * @param path 配置文件路径
     */
    void save(const QString &path) const;
};

} // namespace ClipBridge
