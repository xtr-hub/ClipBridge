#pragma once

#include "register_manager.hpp"
#include "app_config.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

class WindowsRegisterManager : public RegisterManager
{
public:
	WindowsRegisterManager() = default;
	~WindowsRegisterManager() override;

	void register_hotkey(const AppConfig::HotKeyBinding& binding) override;
	void register_hotkeys(const std::vector<AppConfig::HotKeyBinding>& bindings) override;
	void unregister_all() override;

	std::string action_for_id(int id) const override;

private:
	int next_id = 1;
	std::vector<int> register_keys;
	std::unordered_map<int, std::string> actions_by_id;
};
