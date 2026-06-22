#pragma once

#include <string>
#include <memory>

class PathProcesser
{
public:
    virtual ~PathProcesser() = default;

    virtual std::string get_workspace_path() = 0;
    virtual std::string default_path(const std::string& dir_name) = 0;
    virtual std::string normalize_path(const std::string& path) = 0;
    virtual std::string join_paths(const std::string& base, const std::string& relative) = 0;
    virtual bool ensure_directory_exists(const std::string& path) = 0;
    virtual std::string get_config_directory() = 0;

    // 工厂方法
    static std::unique_ptr<PathProcesser> create();
};
