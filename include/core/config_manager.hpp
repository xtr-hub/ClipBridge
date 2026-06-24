#pragma once

#include "app_config.hpp"

#include <string>

class ConfigManager
{
public:
	ConfigManager() = default;
	explicit ConfigManager(std::string path);

	AppConfig load();
	void save(const AppConfig& config);
	AppConfig reload();

private:
	std::string config_path = "config.json";
};
