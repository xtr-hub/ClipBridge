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
    m_handlers["clipboard_image_path"] = [this](const AppConfig::Behavior &behavior) { clipboardImagePath(behavior); };
    m_handlers["strip_newlines"] = [this](const AppConfig::Behavior &behavior) { stripNewlines(behavior); };
    m_handlers["clipboard_file_path"] = [this](const AppConfig::Behavior &behavior) { clipboardFilePath(behavior); };
}

void ActionManager::run(const QString &action, const AppConfig::Behavior &behavior)
{
    if (m_handlers.contains(action)) {
        m_handlers[action](behavior);
    } else {
        qWarning() << "Unknown action:" << action;
    }
}

void ActionManager::updateConfig(const AppConfig &config)
{
    m_config = config;
}

QString ActionManager::getOutputFormat(const QString &action) const
{
    if (m_config.output.formats.contains(action)) {
        return m_config.output.formats[action];
    }
    return m_config.output.format;
}

void ActionManager::clipboardImagePath(const AppConfig::Behavior &behavior)
{
    if (!ClipboardHelper::hasImage()) {
        qDebug() << "No image in clipboard";
        return;
    }

    QImage image = ClipboardHelper::getImage();
    if (image.isNull()) {
        qDebug() << "Failed to get image from clipboard";
        return;
    }

    QString saveDir = getImageSaveDir();
    QString filename = generateImageFilename();
    QString fullPath = QDir(saveDir).filePath(filename);

    if (!ClipboardHelper::saveImageToPng(image, fullPath)) {
        qDebug() << "Failed to save image to:" << fullPath;
        return;
    }

    QString outputText = getOutputFormat("clipboard_image_path");
    outputText.replace("{path}", fullPath);
    ClipboardHelper::setText(outputText);

    if (behavior.autoPaste) {
        Simulator::simulatePaste();
    }

    if (behavior.autoSubmit) {
        Simulator::simulateEnter();
    }

    // Restore immediately — QTimer::singleShot(0) defers to next event loop
    // iteration so the paste/simulateEnter keys are processed first.
    QTimer::singleShot(0, [image]() {
        QApplication::clipboard()->setImage(image);
    });

    qDebug() << "Image saved to:" << fullPath;
}

void ActionManager::stripNewlines(const AppConfig::Behavior &behavior)
{
    QString text = ClipboardHelper::getText();
    if (text.isEmpty()) {
        qDebug() << "No text in clipboard";
        return;
    }

    QString result = text;
    result.remove('\n');
    result.remove('\r');

    ClipboardHelper::setText(result);

    if (behavior.autoPaste) {
        Simulator::simulatePaste();
    }

    if (behavior.autoSubmit) {
        Simulator::simulateEnter();
    }

    QTimer::singleShot(0, [text]() {
        QApplication::clipboard()->setText(text);
    });

    qDebug() << "Strip newlines done!";
}

void ActionManager::clipboardFilePath(const AppConfig::Behavior &behavior)
{
    if (!ClipboardHelper::hasUrls()) {
        qDebug() << "No file URLs in clipboard";
        return;
    }

    QStringList paths = ClipboardHelper::getFilePaths();
    if (paths.isEmpty()) {
        qDebug() << "No valid file paths in clipboard";
        return;
    }

    // Save original URLs before overwriting clipboard
    const QMimeData *srcMime = QApplication::clipboard()->mimeData();
    QList<QUrl> savedUrls = srcMime ? srcMime->urls() : QList<QUrl>();

    QString joinedPaths = paths.join('\n');
    QString outputText = getOutputFormat("clipboard_file_path");
    outputText.replace("{path}", joinedPaths);
    ClipboardHelper::setText(outputText);

    if (behavior.autoPaste) {
        Simulator::simulatePaste();
    }

    if (behavior.autoSubmit) {
        Simulator::simulateEnter();
    }

    QTimer::singleShot(0, [savedUrls]() {
        QMimeData *mime = new QMimeData();
        mime->setUrls(savedUrls);
        QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
    });

    qDebug() << "File paths copied:" << joinedPaths;
}

QString ActionManager::getImageSaveDir() const
{
    if (m_config.output.mode == "custom_path" && !m_config.output.dir.isEmpty()) {
        return m_config.output.dir;
    }

    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    return QDir(desktopPath).filePath("ClipBridge Images");
}

QString ActionManager::generateImageFilename() const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    return QString("clip_%1.png").arg(timestamp);
}

} // namespace ClipBridge
