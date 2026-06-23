#include "windows_register_manager.hpp"
#include "hotkey_keys.hpp"
#include <windows.h>
#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    std::string to_lower(std::string value)
    {
        for (char& ch : value)
        {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        return value;
    }
}

WindowsRegisterManager::~WindowsRegisterManager()
{
    unregister_all();
}

void WindowsRegisterManager::register_hotkey(const AppConfig::HotKeyBinding& binding)
{
    static std::unordered_map<std::string, int> keys_str;

    UINT fs_modifiers = MOD_NOREPEAT;
    UINT vk = 0;

    if (binding.action.empty())
    {
        throw std::invalid_argument("Hotkey action cannot be empty");
    }
    if (binding.keys.empty())
    {
        return;
    }

    std::string key_str = "";
    for (std::string key : binding.keys)
    {
        key = to_lower(key);
        key_str += key;
        if (keys_map.find(key) == keys_map.end()) // 绑定普通键
        {
            if (!vk)
            {
                if (key.size() > 1)
                {
                    throw std::invalid_argument("Unknown hotkey: " + key);
                }
                vk = static_cast<UINT>(std::toupper(static_cast<unsigned char>(key[0])));
            }
            else
            {
                throw std::invalid_argument("Hotkey can only contain one primary key");
            }
            continue;
        }
        fs_modifiers = fs_modifiers | keys_map.at(key); // 绑定修饰键
    }

    if(keys_str.find(key_str) != keys_str.end())
    {
        actions_by_id[keys_str[key_str]].push_back(binding.action); // 如果这个键已经被注册过了就不在注册
        return;
    }

    if (!vk)
    {
        throw std::invalid_argument("Hotkey missing primary key");
    }

    int id = next_id++;
    if (!RegisterHotKey(NULL, id, fs_modifiers, vk))
    {
        throw std::runtime_error("Failed to register hotkey");
    }

    keys_str[key_str] = id;
    register_keys.push_back(id);
    actions_by_id[id].push_back(binding.action);
}

void WindowsRegisterManager::register_hotkeys(const std::vector<AppConfig::HotKeyBinding>& bindings)
{
    for (const AppConfig::HotKeyBinding& binding : bindings)
    {
        register_hotkey(binding);
    }
}

void WindowsRegisterManager::unregister_all()
{
    for (int key_id : register_keys)
    {
        UnregisterHotKey(NULL, key_id);
    }
    register_keys.clear();
    actions_by_id.clear();
}

std::vector<std::string> WindowsRegisterManager::action_for_id(int id) const
{
    auto it = actions_by_id.find(id);
    if (it == actions_by_id.end())
    {
        return {};
    }
    return it->second;
}
