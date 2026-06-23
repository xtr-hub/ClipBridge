#pragma once

#include "app_config.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

class RegisterManager
{
public:
	RegisterManager() = default;
	~RegisterManager();

	void register_hotkey(const AppConfig::HotKeyBinding& binding);
	void register_hotkeys(const std::vector<AppConfig::HotKeyBinding>& bindings);
	void unregister_all();

	std::vector<std::string> action_for_id(int id) const;

private:
	int next_id = 1;
	std::vector<int> register_keys;
	std::unordered_map<int, std::vector<std::string>> actions_by_id;
};
