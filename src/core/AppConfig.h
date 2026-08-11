#pragma once

/**
 * @file AppConfig.h
 * @brief 应用程序配置管理
 * @author Your Name
 * @date 2026
 */

#include <QString>
#include <QVector>
#include <QKeySequence>
#include <QHash>
#include <QSet>
#include <QJsonObject>
#include <QJsonArray>

namespace ClipBridge {

struct AppConfig
{
    struct Behavior
    {
        bool autoPaste = true;
        bool autoSubmit = false;
    };

    struct HotKeyBinding
    {
        QString action;
        QKeySequence keySequence;
        Behavior behavior;
    };

    struct Output
    {
        QString format = "{path}";
        QHash<QString, QString> formats;
        QString mode = "workspace";
        QString dir;
        QString pasteKey = "Ctrl+V";
        int pasteDelay = 100;
    };

    struct MonitorConfig
    {
        bool enabled = false;
        QSet<QString> autoActions;
        int longTextThreshold = 500;
        bool showNotification = true;
    };

    struct InputPanelConfig
    {
        bool alwaysOnTop = true;
        bool saveLongTextAsFile = false;
        int windowOpacity = 95;
    };

    QVector<HotKeyBinding> hotkeys;
    Behavior defaultBehavior;
    Output output;
    MonitorConfig monitor;
    InputPanelConfig inputPanel;
    QKeySequence inputPanelHotkey;
    QString language = "zh_CN";
    bool autoStart = false;

    static AppConfig load(const QString &path);
    void save(const QString &path) const;

    static QStringList availableActions();
    static QString actionDisplayName(const QString &action);

private:
    static QJsonObject saveBehavior(const Behavior &b);
    static Behavior loadBehavior(const QJsonObject &obj, const Behavior &defaultValue);
    static QSet<QString> jsonArrayToSet(const QJsonArray &arr);
    static QJsonArray setToJsonArray(const QSet<QString> &s);
};

} // namespace ClipBridge
