#include "clipboard_manager.hpp"
#include "register_manager.hpp"
#include "path_processer.hpp"
#include "windows_clipboard_manager.hpp"
#include "windows_register_manager.hpp"
#include "windows_path_processer.hpp"

std::unique_ptr<ClipboardManager> ClipboardManager::create()
{
    return std::make_unique<WindowsClipboardManager>();
}

std::unique_ptr<RegisterManager> RegisterManager::create()
{
    return std::make_unique<WindowsRegisterManager>();
}

std::unique_ptr<PathProcesser> PathProcesser::create()
{
    return std::make_unique<WindowsPathProcesser>();
}
