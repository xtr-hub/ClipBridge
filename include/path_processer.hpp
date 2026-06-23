#pragma once
#include <string>

class PathProcesser
{
public:
	PathProcesser() = default;
	~PathProcesser() = default;

	std::string get_workspace_path();
	std::string default_path(const std::string& dir_name);
	std::string normalize_path(const std::string& path);
	std::string join_paths(const std::string& base, const std::string& relative);
	bool ensure_directory_exists(const std::string& path);
	std::string get_config_directory();
};
