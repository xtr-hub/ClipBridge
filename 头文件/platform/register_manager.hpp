#pragma once

#include "app_config.hpp"
#include <string>
#include <vector>
#include <memory>

class RegisterManager
{
public:
    virtual ~RegisterManager() = default;

    virtual void register_hotkey(const AppConfig::HotKeyBinding& binding) = 0;
    virtual void register_hotkeys(const std::vector<AppConfig::HotKeyBinding>& bindings) = 0;
    virtual void unregister_all() = 0;

    // Windows 消息循环需要
    virtual std::string action_for_id(int id) const = 0;

    // 工厂方法
    static std::unique_ptr<RegisterManager> create();
};
