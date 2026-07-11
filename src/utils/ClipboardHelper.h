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

    /**
     * @brief 检查剪贴板是否有 URL（文件路径）
     * @return true 如果有 URL
     */
    static bool hasUrls();

    /**
     * @brief 获取剪贴板中的本地文件路径列表
     * @return 文件路径列表
     */
    static QStringList getFilePaths();

    /**
     * @brief 保存当前剪贴板内容为 MIME 数据副本
     * @return MIME 数据副本（调用方负责释放内存）
     */
    static QMimeData *saveClipboard();

    /**
     * @brief 将剪贴板恢复为指定的 MIME 数据
     * @param data 要恢复的数据（所有权转移给剪贴板，调用方不应再使用）
     */
    static void restoreClipboard(QMimeData *data);

};

} // namespace ClipBridge
