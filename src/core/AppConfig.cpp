#include "AppConfig.h"
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
        config.hotkeys.append({
            "clipboard_image_path",
            QKeySequence("Ctrl+Alt+I"),
            {true, false}
        });
        config.hotkeys.append({
            "strip_newlines",
            QKeySequence("Ctrl+Alt+J"),
            {true, false}
        });
        return config;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return config;
    }

    QJsonObject root = doc.object();

    if (root.contains("default_behavior")) {
        QJsonObject behavior = root["default_behavior"].toObject();
        config.defaultBehavior.autoPaste = behavior["auto_paste"].toBool(true);
        config.defaultBehavior.autoSubmit = behavior["auto_submit"].toBool(false);
    } else if (root.contains("behavior")) {
        QJsonObject behavior = root["behavior"].toObject();
        config.defaultBehavior.autoPaste = behavior["auto_paste"].toBool(true);
        config.defaultBehavior.autoSubmit = behavior["auto_submit"].toBool(false);
    }

    if (root.contains("hotkeys")) {
        QJsonArray hotkeysArray = root["hotkeys"].toArray();
        for (const QJsonValue &val : hotkeysArray) {
            QJsonObject obj = val.toObject();
            HotKeyBinding binding;
            binding.action = obj["action"].toString();

            QString keyStr = obj["key"].toString();
            binding.keySequence = QKeySequence(keyStr);

            if (obj.contains("behavior")) {
                QJsonObject behavior = obj["behavior"].toObject();
                binding.behavior.autoPaste = behavior["auto_paste"].toBool(config.defaultBehavior.autoPaste);
                binding.behavior.autoSubmit = behavior["auto_submit"].toBool(config.defaultBehavior.autoSubmit);
            } else {
                binding.behavior = config.defaultBehavior;
            }

            config.hotkeys.append(binding);
        }
    }

    if (root.contains("output")) {
        QJsonObject output = root["output"].toObject();
        config.output.format = output["format"].toString("{path}");
        config.output.mode = output["mode"].toString("workspace");
        config.output.dir = output["dir"].toString();
    }

    return config;
}

void AppConfig::save(const QString &path) const
{
    QJsonObject root;

    QJsonArray hotkeysArray;
    for (const auto &binding : hotkeys) {
        QJsonObject obj;
        obj["action"] = binding.action;
        obj["key"] = binding.keySequence.toString(QKeySequence::PortableText);

        QJsonObject behaviorObj;
        behaviorObj["auto_paste"] = binding.behavior.autoPaste;
        behaviorObj["auto_submit"] = binding.behavior.autoSubmit;
        obj["behavior"] = behaviorObj;

        hotkeysArray.append(obj);
    }
    root["hotkeys"] = hotkeysArray;

    QJsonObject defaultBehaviorObj;
    defaultBehaviorObj["auto_paste"] = defaultBehavior.autoPaste;
    defaultBehaviorObj["auto_submit"] = defaultBehavior.autoSubmit;
    root["default_behavior"] = defaultBehaviorObj;

    QJsonObject outputObj;
    outputObj["format"] = output.format;
    outputObj["mode"] = output.mode;
    outputObj["dir"] = output.dir;
    root["output"] = outputObj;

    QJsonDocument doc(root);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson(QJsonDocument::Indented));
    }
}

} // namespace ClipBridge
