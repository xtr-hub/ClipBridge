#pragma once

#include "app_config.hpp"

#include <string>
#include <unordered_map>
#include <stdexcept>

class ActionManager
{
public:
	explicit ActionManager(const AppConfig& config) : config(config) {}
	void update_config(const AppConfig& new_config) { config = new_config; }
	void run(const std::string& action, const AppConfig::Behavior& behavior);

private:
	using Handler = void (ActionManager::*)(const AppConfig::Behavior&);

	AppConfig config;

	static const std::unordered_map<std::string, Handler> handlers;

	void clipboard_image_path(const AppConfig::Behavior& behavior);

	void strip_newlines(const AppConfig::Behavior& behavior);
};
