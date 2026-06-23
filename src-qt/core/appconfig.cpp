#include "appconfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDir>

namespace ClipBridge {

AppConfig AppConfig::load(const QString &path)
{
    AppConfig config;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 如果配置文件不存在，使用默认配置
        config.hotkeys.append({
            "clipboard_image_path",
            QKeySequence("Ctrl+Alt+I")
        });
        config.hotkeys.append({
            "strip_newlines",
            QKeySequence("Ctrl+Alt+J")
        });
        return config;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return config;
    }

    QJsonObject root = doc.object();

    // 读取热键
    if (root.contains("hotkeys")) {
        QJsonArray hotkeysArray = root["hotkeys"].toArray();
        for (const QJsonValue &val : hotkeysArray) {
            QJsonObject obj = val.toObject();
            HotKeyBinding binding;
            binding.action = obj["action"].toString();

            QString keyStr = obj["key"].toString();
            binding.keySequence = QKeySequence(keyStr);

            config.hotkeys.append(binding);
        }
    }

    // 读取 behavior
    if (root.contains("behavior")) {
        QJsonObject behavior = root["behavior"].toObject();
        config.behavior.autoPaste = behavior["auto_paste"].toBool(true);
        config.behavior.autoSubmit = behavior["auto_submit"].toBool(false);
    }

    // 读取 output
    if (root.contains("output")) {
        QJsonObject output = root["output"].toObject();
        config.output.format = output["format"].toString("{path}");

        if (output.contains("path")) {
            QJsonObject pathObj = output["path"].toObject();
            config.output.mode = pathObj["mode"].toString("workspace");
            config.output.dir = pathObj["dir"].toString();
        }
    }

    return config;
}

void AppConfig::save(const QString &path) const
{
    QJsonObject root;

    // 保存热键
    QJsonArray hotkeysArray;
    for (const auto &binding : hotkeys) {
        QJsonObject obj;
        obj["action"] = binding.action;
        obj["key"] = binding.keySequence.toString(QKeySequence::PortableText);
        hotkeysArray.append(obj);
    }
    root["hotkeys"] = hotkeysArray;

    // 保存 behavior
    QJsonObject behaviorObj;
    behaviorObj["auto_paste"] = behavior.autoPaste;
    behaviorObj["auto_submit"] = behavior.autoSubmit;
    root["behavior"] = behaviorObj;

    // 保存 output
    QJsonObject outputObj;
    outputObj["format"] = output.format;

    QJsonObject pathObj;
    pathObj["mode"] = output.mode;
    pathObj["dir"] = output.dir;
    outputObj["path"] = pathObj;

    root["output"] = outputObj;

    QJsonDocument doc(root);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson(QJsonDocument::Indented));
    }
}

} // namespace ClipBridge
