#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <utility>

class ClipboardManager
{
public:
    ClipboardManager();
    ~ClipboardManager();

    // ClipboardManager 接口实现
    std::string get_text();
    void set_text(const std::string& text);
    bool has_image();
    std::vector<std::string> get_available_image_formats();
    bool save_image_to_png(const std::string& file_path);
    void simulate_paste();

private:
    static bool gdiplus_initialized;
    static ULONG_PTR gdiplus_token;
    static int instance_count;

    static void ensure_gdiplus_initialized();
    static void shutdown_gdiplus();

    static std::wstring get_text_wide();
    static std::vector<UINT> get_available_image_formats_win32();
    static std::pair<void*, size_t> get_dib();
    static void free_dib(void* dib_data);
    static void save_dib_to_png(void* dib_data, size_t dib_size, const std::wstring& file_path);
};
