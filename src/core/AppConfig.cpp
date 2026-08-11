#include "AppConfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDir>

namespace ClipBridge {

QJsonObject AppConfig::saveBehavior(const Behavior &b)
{
    QJsonObject obj;
    obj["auto_paste"] = b.autoPaste;
    obj["auto_submit"] = b.autoSubmit;
    return obj;
}

AppConfig::Behavior AppConfig::loadBehavior(const QJsonObject &obj, const Behavior &defaultValue)
{
    Behavior b;
    b.autoPaste = obj["auto_paste"].toBool(defaultValue.autoPaste);
    b.autoSubmit = obj["auto_submit"].toBool(defaultValue.autoSubmit);
    return b;
}

QSet<QString> AppConfig::jsonArrayToSet(const QJsonArray &arr)
{
    QSet<QString> result;
    for (const QJsonValue &v : arr)
        result.insert(v.toString());
    return result;
}

QJsonArray AppConfig::setToJsonArray(const QSet<QString> &s)
{
    QJsonArray arr;
    for (const QString &item : s)
        arr.append(item);
    return arr;
}

QStringList AppConfig::availableActions()
{
    return {"clipboard_image_path", "clipboard_file_path",
            "strip_newlines", "clipboard_long_text"};
}

QString AppConfig::actionDisplayName(const QString &action)
{
    if (action == "clipboard_image_path")  return QObject::tr("复制图片路径");
    if (action == "clipboard_file_path")   return QObject::tr("复制文件路径");
    if (action == "strip_newlines")        return QObject::tr("去除换行符");
    if (action == "clipboard_long_text")   return QObject::tr("长文本转文件路径");
    return action;
}

AppConfig AppConfig::load(const QString &path)
{
    AppConfig config;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        config.hotkeys.append({"clipboard_image_path",
                               QKeySequence("Ctrl+Alt+I"), {true, false}});
        config.hotkeys.append({"strip_newlines",
                               QKeySequence("Ctrl+Alt+J"), {true, false}});
        config.hotkeys.append({"clipboard_file_path",
                               QKeySequence("Ctrl+Alt+F"), {true, false}});
        config.inputPanelHotkey = QKeySequence("Ctrl+Alt+Space");
        return config;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return config;

    QJsonObject root = doc.object();

    // default_behavior
    if (root.contains("default_behavior"))
        config.defaultBehavior = loadBehavior(root["default_behavior"].toObject(), config.defaultBehavior);
    else if (root.contains("behavior"))
        config.defaultBehavior = loadBehavior(root["behavior"].toObject(), config.defaultBehavior);

    // hotkeys
    if (root.contains("hotkeys")) {
        QJsonArray arr = root["hotkeys"].toArray();
        for (const QJsonValue &val : arr) {
            QJsonObject obj = val.toObject();
            HotKeyBinding binding;
            binding.action = obj["action"].toString();
            binding.keySequence = QKeySequence(obj["key"].toString());
            if (obj.contains("behavior"))
                binding.behavior = loadBehavior(obj["behavior"].toObject(), config.defaultBehavior);
            else
                binding.behavior = config.defaultBehavior;
            config.hotkeys.append(binding);
        }
    }

    // output
    if (root.contains("output")) {
        QJsonObject out = root["output"].toObject();
        config.output.format = out["format"].toString("{path}");
        config.output.mode = out["mode"].toString("workspace");
        config.output.dir = out["dir"].toString();
        if (out.contains("formats") && out["formats"].isObject()) {
            QJsonObject fmts = out["formats"].toObject();
            for (const QString &k : fmts.keys())
                config.output.formats[k] = fmts[k].toString();
        }
        config.output.pasteKey = out["paste_key"].toString("Ctrl+V");
        config.output.pasteDelay = out["paste_delay"].toInt(100);
    }

    // monitor
    if (root.contains("monitor")) {
        QJsonObject mon = root["monitor"].toObject();
        config.monitor.enabled = mon["enabled"].toBool(false);
        config.monitor.autoActions = jsonArrayToSet(mon["auto_actions"].toArray());
        config.monitor.longTextThreshold = mon["long_text_threshold"].toInt(500);
        config.monitor.showNotification = mon["show_notification"].toBool(true);
    }

    // input_panel
    if (root.contains("input_panel")) {
        QJsonObject ip = root["input_panel"].toObject();
        config.inputPanel.alwaysOnTop = ip["always_on_top"].toBool(true);
        config.inputPanel.saveLongTextAsFile = ip["save_long_text_as_file"].toBool(false);
        config.inputPanel.windowOpacity = ip["opacity"].toInt(95);
    }

    // input_panel_hotkey
    if (root.contains("input_panel_hotkey"))
        config.inputPanelHotkey = QKeySequence(root["input_panel_hotkey"].toString());

    // auto_start
    if (root.contains("auto_start"))
        config.autoStart = root["auto_start"].toBool(false);

    // language
    if (root.contains("language"))
        config.language = root["language"].toString("zh_CN");

    return config;
}

void AppConfig::save(const QString &path) const
{
    QJsonObject root;

    // hotkeys
    QJsonArray hotkeysArr;
    for (const auto &b : hotkeys) {
        QJsonObject obj;
        obj["action"] = b.action;
        obj["key"] = b.keySequence.toString(QKeySequence::PortableText);
        obj["behavior"] = saveBehavior(b.behavior);
        hotkeysArr.append(obj);
    }
    root["hotkeys"] = hotkeysArr;

    // default_behavior
    root["default_behavior"] = saveBehavior(defaultBehavior);

    // output
    QJsonObject out;
    out["format"] = output.format;
    out["mode"] = output.mode;
    out["dir"] = output.dir;
    QJsonObject fmts;
    for (auto it = output.formats.cbegin(); it != output.formats.cend(); ++it)
        fmts[it.key()] = it.value();
    out["formats"] = fmts;
    out["paste_key"] = output.pasteKey;
    out["paste_delay"] = output.pasteDelay;
    root["output"] = out;

    // monitor
    QJsonObject mon;
    mon["enabled"] = monitor.enabled;
    mon["auto_actions"] = setToJsonArray(monitor.autoActions);
    mon["long_text_threshold"] = monitor.longTextThreshold;
    mon["show_notification"] = monitor.showNotification;
    root["monitor"] = mon;

    // input_panel
    QJsonObject ip;
    ip["always_on_top"] = inputPanel.alwaysOnTop;
    ip["save_long_text_as_file"] = inputPanel.saveLongTextAsFile;
    ip["opacity"] = inputPanel.windowOpacity;
    root["input_panel"] = ip;

    // input_panel_hotkey
    root["input_panel_hotkey"] = inputPanelHotkey.toString(QKeySequence::PortableText);

    // auto_start
    root["auto_start"] = autoStart;

    // language
    root["language"] = language;

    QJsonDocument doc(root);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        file.write(doc.toJson(QJsonDocument::Indented));
}

} // namespace ClipBridge
