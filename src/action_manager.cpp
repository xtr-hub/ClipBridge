#include "action_manager.hpp"
#include "clipboard_manager.hpp"
#include "path_processer.hpp"
#include <unordered_map>
#include <stdexcept>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>

const std::unordered_map<std::string, ActionManager::Handler> ActionManager::handlers = {
    {"clipboard_image_path", &ActionManager::clipboard_image_path},
    {"strip_newlines", &ActionManager::strip_newlines}
};

void ActionManager::run(const std::string& action)
{
    auto it = handlers.find(action);
    if (it == handlers.end()) {
        throw std::invalid_argument("Unknown action: " + action);
    }
    (this->*(it->second))();
}

void ActionManager::clipboard_image_path()
{
    ClipboardManager clipboard;
    if (!clipboard.has_image()) {
        return;
    }

    PathProcesser path_proc;

    std::string output_dir;
    switch (config.output.path.mode) {
        case AppConfig::Output::Path::Mode::workspace:
            output_dir = path_proc.get_workspace_path();
            break;
        case AppConfig::Output::Path::Mode::custom_path:
            output_dir = config.output.path.dir;
            break;
    }

    if (output_dir.empty()) {
        output_dir = "ClipBridge";
    }

    if (!path_proc.ensure_directory_exists(output_dir)) {
        throw std::runtime_error("Failed to create output directory");
    }

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm local_time;
    localtime_s(&local_time, &time_t_now);

    std::ostringstream filename;
    filename << "clip_"
        << std::put_time(&local_time, "%Y%m%d_%H%M%S")
        << "_" << std::setfill('0') << std::setw(3) << ms.count()
        << ".png";

    std::string full_path = path_proc.join_paths(output_dir, filename.str());

    bool success = false;
    try {
        success = clipboard.save_image_to_png(full_path);
    } catch (std::exception e){
        throw std::runtime_error(e.what());
    }

    if (!success) {
        throw std::runtime_error("Failed to save clipboard image");
    }

    std::string clipboard_content = config.output.format;
    size_t pos = clipboard_content.find("{path}");
    if (pos != std::string::npos) {
        clipboard_content.replace(pos, 6, full_path);
    }
    if (!clipboard_content.empty()) {
        clipboard.set_text(clipboard_content);
        if(config.behavior.auto_paste){
            clipboard.simulate_paste();
        }
    }
}

void ActionManager::strip_newlines()
{
    ClipboardManager clipboard;
    std::string clipboard_text;
    try{
        clipboard_text = clipboard.get_text();
    } catch(std::exception e){
        throw std::runtime_error(e.what());
    }
    if(clipboard_text.empty()) return;
    clipboard_text.erase(std::remove(clipboard_text.begin(), clipboard_text.end(), '\n'), clipboard_text.end());
    clipboard_text.erase(std::remove(clipboard_text.begin(), clipboard_text.end(), '\r'), clipboard_text.end());
    try{
        clipboard.set_text(clipboard_text);
    } catch(std::exception e){
        throw std::runtime_error(e.what());
    }
    if(config.behavior.auto_paste){
        clipboard.simulate_paste();
    }
}
