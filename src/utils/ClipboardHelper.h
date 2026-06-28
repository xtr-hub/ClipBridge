#pragma once

/**
 * @file clipboardhelper.h
 * @brief 剪贴板工具类
 * @author Your Name
 * @date 2026
 */

#include <QString>
#include <QImage>
#include <QMimeData>

namespace ClipBridge {

/**
 * @brief 剪贴板辅助工具
 */
class ClipboardHelper
{
public:
    /**
     * @brief 检查剪贴板是否有图片
     * @return true 如果有图片
     */
    static bool hasImage();

    /**
     * @brief 获取剪贴板中的图片
     * @return 图片对象
     */
    static QImage getImage();

    /**
     * @brief 保存图片到 PNG 文件
     * @param image 图片
     * @param path 文件路径
     * @return true 保存成功
     */
    static bool saveImageToPng(const QImage &image, const QString &path);

    /**
     * @brief 获取剪贴板文本
     * @return 文本内容
     */
    static QString getText();

    /**
     * @brief 设置剪贴板文本
     * @param text 文本内容
     */
    static void setText(const QString &text);
    
};

} // namespace ClipBridge
