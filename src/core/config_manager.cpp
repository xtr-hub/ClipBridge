#include "config_manager.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <windows.h>

using json = nlohmann::json;

namespace
{
    std::vector<std::string> split_string(const std::string& str, char separator)
    {
        std::vector<std::string> result;
        std::string current;

        for (char ch : str)
        {
            if (ch == separator)
            {
                if (!current.empty())
                {
                    result.push_back(current);
                    current.clear();
                }
            }
            else if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
            {
                current.push_back(ch);
            }
        }

        if (!current.empty())
        {
            result.push_back(current);
        }

        return result;
    }

    template<typename T>
    T read(const json& j, const char* key, const T& default_value)
    {
        auto it = j.find(key);
        if (it == j.end() || it->is_null())
        {
            return default_value;
        }

        try
        {
            return it->get<T>();
        }
        catch (const json::exception&)
        {
            return default_value;
        }
    }

    template<typename T>
    void write(json& j, const char* key, const T& value)
    {
        j[key] = value;
    }

    std::string get_keys(const std::vector<std::string>& keys) {
        std::string keys_str;
        for (size_t i = 0; i < keys.size(); ++i)
        {
            if (i != 0)
            {
                keys_str.append("+");
            }
            keys_str.append(keys[i]);
        }
        return keys_str;
    }

    void load_hotkeys(const json& j, AppConfig& config)
    {
        if (j.contains("hotkeys") && j.at("hotkeys").is_array())
        {
            std::vector<AppConfig::HotKeyBinding> hotkeys;
            for (const json& item : j.at("hotkeys"))
            {
                if (!item.is_object())
                {
                    continue;
                }

                AppConfig::HotKeyBinding binding;
                binding.action = read(item, "action", binding.action);
                std::string key = read(item, "key", get_keys(binding.keys));
                binding.keys = split_string(key, '+');

                if (!binding.action.empty() && !binding.keys.empty())
                {
                    hotkeys.push_back(binding);
                }
            }

            if (!hotkeys.empty())
            {
                config.hotkeys = hotkeys;
            }
            return;
        }

        if (!j.contains("hotkey"))
        {
            return;
        }

        const json& hotkey = j.at("hotkey");
        AppConfig::HotKeyBinding binding;
        std::string key = read(hotkey, "key", get_keys(binding.keys));
        binding.keys = split_string(key, '+');
        config.hotkeys = { binding };
    }

    void load_behavior(const json& j, AppConfig& config)
    {
        if (!j.contains("behavior"))
        {
            return;
        }

        const json& behavior = j.at("behavior");
        config.behavior.auto_paste = read(behavior, "auto_paste", config.behavior.auto_paste);
        config.behavior.auto_submit = read(behavior, "auto_submit", config.behavior.auto_submit);
    }

    void load_output_path(const json& output, AppConfig& config)
    {
        if (!output.contains("path"))
        {
            return;
        }

        const json& path = output.at("path");

        std::string mode = read(path, "mode", AppConfig::Output::Path::to_string(config.output.path.mode));
        if (auto parsed_mode = AppConfig::Output::Path::mode_from_string(mode))
        {
            config.output.path.mode = *parsed_mode;
        }

        config.output.path.dir = read(path, "dir", config.output.path.dir);
    }

    void load_output(const json& j, AppConfig& config)
    {
        if (!j.contains("output"))
        {
            return;
        }

        const json& output = j.at("output");

        std::string mode = read(output, "mode", AppConfig::Output::to_string(config.output.mode));
        if (auto parsed_mode = AppConfig::Output::mode_from_string(mode))
        {
            config.output.mode = *parsed_mode;
        }

        config.output.format = read(output, "format", config.output.format);

        load_output_path(output, config);
    }

    void load_system(const json& j, AppConfig& config)
    {
        std::string system_str = read(j, "system", AppConfig::to_string(config.system));
        if (auto system = AppConfig::system_from_string(system_str))
        {
            config.system = *system;
        }
    }

    bool write_json_safely(const std::string& config_path, const json& j)
    {
        std::filesystem::path target_path(config_path);
        std::filesystem::path temp_path = target_path;
        temp_path += ".tmp";

        std::error_code ec;
        std::filesystem::path parent_path = target_path.parent_path();
        if (!parent_path.empty())
        {
            std::filesystem::create_directories(parent_path, ec);
            if (ec)
            {
                return false;
            }
        }

        {
            std::ofstream file(temp_path, std::ios::binary | std::ios::trunc);
            if (!file)
            {
                return false;
            }

            std::string content = j.dump(4);
            content.push_back('\n');
            file.write(content.data(), static_cast<std::streamsize>(content.size()));
            file.flush();

            if (!file)
            {
                return false;
            }
        }

        if (!MoveFileExW(
            temp_path.wstring().c_str(),
            target_path.wstring().c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        {
            std::filesystem::remove(temp_path, ec);
            return false;
        }

        return true;
    }
}

AppConfig ConfigManager::load()
{
    AppConfig config;

    std::ifstream file(config_path);
    if (!file)
    {
        return config;
    }

    json j;
    file >> j;

    load_hotkeys(j, config);
    load_behavior(j, config);
    load_system(j, config);
    load_output(j, config);

    return config;
}

void ConfigManager::save(const AppConfig& config)
{
    json j;

    json hotkeys = json::array();
    for (const AppConfig::HotKeyBinding& binding : config.hotkeys)
    {
        json hotkey;
        write(hotkey, "action", binding.action);
        write(hotkey, "key", get_keys(binding.keys));
        hotkeys.push_back(hotkey);
    }
    j["hotkeys"] = hotkeys;

    json behavior;
    write(behavior, "auto_paste", config.behavior.auto_paste);
    write(behavior, "auto_submit", config.behavior.auto_submit);
    j["behavior"] = behavior;

    write(j, "system", AppConfig::to_string(config.system));

    json path;
    write(path, "mode", AppConfig::Output::Path::to_string(config.output.path.mode));
    write(path, "dir", config.output.path.dir);

    json output;
    write(output, "mode", AppConfig::Output::to_string(config.output.mode));
    write(output, "format", config.output.format);
    output["path"] = path;
    j["output"] = output;

    write_json_safely(config_path, j);
}

AppConfig ConfigManager::reload()
{
    return load();
}
