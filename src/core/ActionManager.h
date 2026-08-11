#pragma once

#include "AppConfig.h"
#include <QString>
#include <QHash>
#include <functional>

namespace ClipBridge {

class ActionManager
{
public:
    explicit ActionManager(const AppConfig &config);

    void run(const QString &action, const AppConfig::Behavior &behavior);
    QString process(const QString &action);
    void updateConfig(const AppConfig &config);
    int longTextThreshold() const;

    // Returns true if this action can process the given clipboard content type.
    // Centralises action-contentType knowledge so callers (MonitorService,
    // InputPanel) don't duplicate the mapping.
    static bool canHandleAction(const QString &action, const QString &contentType);

private:
    AppConfig m_config;
    QHash<QString, std::function<void(const AppConfig::Behavior &)>> m_handlers;

    void clipboardImagePath(const AppConfig::Behavior &behavior);
    void stripNewlines(const AppConfig::Behavior &behavior);
    void clipboardFilePath(const AppConfig::Behavior &behavior);
    void clipboardLongText(const AppConfig::Behavior &behavior);

    QString getOutputFormat(const QString &action) const;
    QString getImageSaveDir() const;
    QString getTextSaveDir() const;
    QString generateImageFilename() const;
    QString generateTextFilename() const;
};

} // namespace ClipBridge
