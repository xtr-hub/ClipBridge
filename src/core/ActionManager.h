#pragma once

/**
 * @file ActionManager.h
 * @brief 动作管理器
 * @author Your Name
 * @date 2026
 */

#include "AppConfig.h"
#include <QString>
#include <QHash>
#include <functional>

namespace ClipBridge {

/**
 * @brief 动作管理器，负责执行各种剪贴板操作
 */
class ActionManager
{
public:
    /**
     * @brief 构造函数
     * @param config 应用配置
     */
    explicit ActionManager(const AppConfig &config);

    /**
     * @brief 执行动作
     * @param action 动作名称
     */
    void run(const QString &action);

private:
    AppConfig m_config;
    QHash<QString, std::function<void()>> m_handlers;

    /**
     * @brief 将剪贴板图片保存为文件路径
     */
    void clipboardImagePath();

    /**
     * @brief 去除剪贴板文本中的换行符
     */
    void stripNewlines();

    /**
     * @brief 获取图片保存目录
     * @return 目录路径
     */
    QString getImageSaveDir() const;

    /**
     * @brief 生成唯一的图片文件名
     * @return 文件名
     */
    QString generateImageFilename() const;
};

} // namespace ClipBridge
