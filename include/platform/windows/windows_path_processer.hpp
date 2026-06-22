#pragma once
#include "path_processer.hpp"
#include <string>

class WindowsPathProcesser : public PathProcesser
{
public:
	WindowsPathProcesser() = default;
	~WindowsPathProcesser() override = default;

	std::string get_workspace_path() override;
	std::string default_path(const std::string& dir_name) override;
	std::string normalize_path(const std::string& path) override;
	std::string join_paths(const std::string& base, const std::string& relative) override;
	bool ensure_directory_exists(const std::string& path) override;
	std::string get_config_directory() override;
};
