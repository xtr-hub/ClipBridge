#include "ActionManager.h"
#include "ClipboardHelper.h"
#include "Simulator.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include <QDebug>

namespace ClipBridge {

ActionManager::ActionManager(const AppConfig &config)
    : m_config(config)
{
    m_handlers["clipboard_image_path"] = [this](const AppConfig::Behavior &b) { clipboardImagePath(b); };
    m_handlers["strip_newlines"]        = [this](const AppConfig::Behavior &b) { stripNewlines(b); };
    m_handlers["clipboard_file_path"]   = [this](const AppConfig::Behavior &b) { clipboardFilePath(b); };
    m_handlers["clipboard_long_text"]   = [this](const AppConfig::Behavior &b) { clipboardLongText(b); };
}

void ActionManager::run(const QString &action, const AppConfig::Behavior &behavior)
{
    if (m_handlers.contains(action))
        m_handlers[action](behavior);
    else
        qWarning() << "Unknown action:" << action;
}

QString ActionManager::process(const QString &action)
{
    if (action == "clipboard_image_path") {
        if (!ClipboardHelper::hasImage()) return {};
        QImage image = ClipboardHelper::getImage();
        if (image.isNull()) return {};
        QString fullPath = QDir(getImageSaveDir()).filePath(generateImageFilename());
        if (!ClipboardHelper::saveImageToPng(image, fullPath)) return {};
        QString out = getOutputFormat(action);
        out.replace("{path}", fullPath);
        return out;
    }
    if (action == "clipboard_file_path") {
        QStringList paths = ClipboardHelper::getFilePaths();
        if (paths.isEmpty()) return {};
        QString out = getOutputFormat(action);
        out.replace("{path}", paths.join('\n'));
        return out;
    }
    if (action == "strip_newlines") {
        QString text = ClipboardHelper::getText();
        text.remove('\n');
        text.remove('\r');
        return text;
    }
    if (action == "clipboard_long_text") {
        QString text = ClipboardHelper::getText();
        if (text.isEmpty()) return {};
        int threshold = m_config.monitor.longTextThreshold;
        if (threshold > 0 && text.length() > threshold) {
            QString fullPath = QDir(getTextSaveDir()).filePath(generateTextFilename());
            if (ClipboardHelper::saveTextToFile(text, fullPath)) {
                QString out = getOutputFormat(action);
                out.replace("{path}", fullPath);
                out.replace("{text}", text);
                return out;
            }
        }
        QString out = getOutputFormat(action);
        out.replace("{path}", text);
        out.replace("{text}", text);
        return out;
    }
    return {};
}

void ActionManager::updateConfig(const AppConfig &config)
{
    m_config = config;
}

int ActionManager::longTextThreshold() const
{
    return m_config.monitor.longTextThreshold;
}

QString ActionManager::getOutputFormat(const QString &action) const
{
    if (m_config.output.formats.contains(action))
        return m_config.output.formats[action];
    return m_config.output.format;
}

void ActionManager::clipboardImagePath(const AppConfig::Behavior &behavior)
{
    if (!ClipboardHelper::hasImage()) { qDebug() << "No image in clipboard"; return; }
    QImage image = ClipboardHelper::getImage();
    if (image.isNull()) { qDebug() << "Failed to get image"; return; }

    QString fullPath = QDir(getImageSaveDir()).filePath(generateImageFilename());
    if (!ClipboardHelper::saveImageToPng(image, fullPath)) {
        qDebug() << "Failed to save image to:" << fullPath; return;
    }

    QString out = getOutputFormat("clipboard_image_path");
    out.replace("{path}", fullPath);
    ClipboardHelper::setText(out);

    if (behavior.autoPaste)
        Simulator::simulatePaste(m_config.output.pasteKey);
    if (behavior.autoSubmit)
        Simulator::simulateEnter();

    QTimer::singleShot(m_config.output.pasteDelay, [image]() {
        QApplication::clipboard()->setImage(image);
    });
    qDebug() << "Image saved to:" << fullPath;
}

void ActionManager::stripNewlines(const AppConfig::Behavior &behavior)
{
    QString text = ClipboardHelper::getText();
    if (text.isEmpty()) { qDebug() << "No text in clipboard"; return; }

    QString result = text;
    result.remove('\n');
    result.remove('\r');
    ClipboardHelper::setText(result);

    if (behavior.autoPaste)
        Simulator::simulatePaste(m_config.output.pasteKey);
    if (behavior.autoSubmit)
        Simulator::simulateEnter();

    QTimer::singleShot(m_config.output.pasteDelay, [text]() {
        QApplication::clipboard()->setText(text);
    });
    qDebug() << "Strip newlines done";
}

void ActionManager::clipboardFilePath(const AppConfig::Behavior &behavior)
{
    if (!ClipboardHelper::hasUrls()) { qDebug() << "No file URLs in clipboard"; return; }
    QStringList paths = ClipboardHelper::getFilePaths();
    if (paths.isEmpty()) { qDebug() << "No valid file paths"; return; }

    const QMimeData *srcMime = QApplication::clipboard()->mimeData();
    QList<QUrl> savedUrls = srcMime ? srcMime->urls() : QList<QUrl>();

    QString out = getOutputFormat("clipboard_file_path");
    out.replace("{path}", paths.join('\n'));
    ClipboardHelper::setText(out);

    if (behavior.autoPaste)
        Simulator::simulatePaste(m_config.output.pasteKey);
    if (behavior.autoSubmit)
        Simulator::simulateEnter();

    QTimer::singleShot(m_config.output.pasteDelay, [savedUrls]() {
        QMimeData *mime = new QMimeData();
        mime->setUrls(savedUrls);
        QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
    });
    qDebug() << "File paths copied:" << paths;
}

void ActionManager::clipboardLongText(const AppConfig::Behavior &behavior)
{
    QString text = ClipboardHelper::getText();
    if (text.isEmpty()) { qDebug() << "No text in clipboard"; return; }

    QString originalText = text;
    int threshold = m_config.monitor.longTextThreshold;
    QString out;

    if (threshold > 0 && text.length() > threshold) {
        QString saveDir = getTextSaveDir();
        QString fullPath = QDir(saveDir).filePath(generateTextFilename());
        if (!ClipboardHelper::saveTextToFile(text, fullPath)) {
            qDebug() << "Failed to save text file";
            return;
        }
        out = getOutputFormat("clipboard_long_text");
        out.replace("{path}", fullPath);
        out.replace("{text}", originalText);
        qDebug() << "Long text saved to:" << fullPath;
    } else {
        out = getOutputFormat("clipboard_long_text");
        out.replace("{path}", originalText);
        out.replace("{text}", originalText);
    }

    ClipboardHelper::setText(out);

    if (behavior.autoPaste)
        Simulator::simulatePaste(m_config.output.pasteKey);
    if (behavior.autoSubmit)
        Simulator::simulateEnter();

    QTimer::singleShot(m_config.output.pasteDelay, [originalText]() {
        QApplication::clipboard()->setText(originalText);
    });
}

QString ActionManager::getImageSaveDir() const
{
    if (m_config.output.mode == "custom_path" && !m_config.output.dir.isEmpty())
        return m_config.output.dir;
    QString temp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return QDir(temp).filePath("ClipBridge Images");
}

QString ActionManager::getTextSaveDir() const
{
    if (m_config.output.mode == "custom_path" && !m_config.output.dir.isEmpty())
        return m_config.output.dir;
    QString temp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return QDir(temp).filePath("ClipBridge Texts");
}

QString ActionManager::generateImageFilename() const
{
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    return QString("clip_%1.png").arg(ts);
}

QString ActionManager::generateTextFilename() const
{
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    return QString("clip_%1.txt").arg(ts);
}

bool ActionManager::canHandleAction(const QString &action, const QString &contentType)
{
    if (action == "clipboard_image_path") return contentType == "image";
    if (action == "clipboard_file_path")  return contentType == "files";
    if (action == "strip_newlines")       return contentType == "text";
    if (action == "clipboard_long_text")  return contentType == "text";
    return false;
}

} // namespace ClipBridge
