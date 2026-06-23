#include "windows_clipboard_manager.hpp"
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <stdexcept>
#include <utility>

#pragma comment(lib, "gdiplus.lib")

// 静态成员初始化
bool WindowsClipboardManager::gdiplus_initialized = false;
ULONG_PTR WindowsClipboardManager::gdiplus_token = 0;
int WindowsClipboardManager::instance_count = 0;

// UTF-16 转 UTF-8
static std::string utf16_to_utf8(const std::wstring& wstr)
{
    if (wstr.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    if (size <= 0) return {};
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], size, nullptr, nullptr);
    return result;
}

// UTF-8 转 UTF-16
static std::wstring utf8_to_utf16(const std::string& str)
{
    if (str.empty()) return {};
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    if (size <= 0) return {};
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], size);
    return result;
}

WindowsClipboardManager::WindowsClipboardManager()
{
    if (instance_count++ == 0) {
        ensure_gdiplus_initialized();
    }
}

WindowsClipboardManager::~WindowsClipboardManager()
{
    if (--instance_count == 0) {
        shutdown_gdiplus();
    }
}

void WindowsClipboardManager::ensure_gdiplus_initialized()
{
    if (gdiplus_initialized) return;

    Gdiplus::GdiplusStartupInput startup_input;
    Gdiplus::GdiplusStartupOutput startup_output;
    Gdiplus::Status status = Gdiplus::GdiplusStartup(&gdiplus_token, &startup_input, &startup_output);

    if (status != Gdiplus::Ok)
        throw std::runtime_error("Failed to initialize GDI+");

    gdiplus_initialized = true;
}

void WindowsClipboardManager::shutdown_gdiplus()
{
    if (gdiplus_initialized) {
        Gdiplus::GdiplusShutdown(gdiplus_token);
        gdiplus_initialized = false;
    }
}

std::wstring WindowsClipboardManager::get_text_wide()
{
    if (!OpenClipboard(nullptr))
        throw std::runtime_error("Failed to open clipboard");

    HANDLE data_handle = GetClipboardData(CF_UNICODETEXT);
    wchar_t* text_ptr = nullptr;
    if (data_handle != nullptr)
        text_ptr = static_cast<wchar_t*>(GlobalLock(data_handle));

    std::wstring result = L"";
    if (text_ptr != nullptr)
        result = std::wstring(text_ptr);

    if (data_handle != nullptr)
        GlobalUnlock(data_handle);

    CloseClipboard();
    return result;
}

std::string WindowsClipboardManager::get_text()
{
    return utf16_to_utf8(get_text_wide());
}

void WindowsClipboardManager::set_text(const std::string& text)
{
    if (!OpenClipboard(nullptr))
        throw std::runtime_error("Failed to open clipboard");

    EmptyClipboard();

    std::wstring wtext = utf8_to_utf16(text);
    size_t byte_size = (wtext.size() + 1) * sizeof(wchar_t);
    HANDLE data_handle = GlobalAlloc(GMEM_MOVEABLE, byte_size);

    if (data_handle != nullptr) {
        wchar_t* dest = static_cast<wchar_t*>(GlobalLock(data_handle));
        if (dest != nullptr) {
            memcpy(dest, wtext.c_str(), byte_size);
            GlobalUnlock(data_handle);
            SetClipboardData(CF_UNICODETEXT, data_handle);
        }
    }

    CloseClipboard();
}

std::vector<UINT> WindowsClipboardManager::get_available_image_formats_win32()
{
    if (!OpenClipboard(nullptr))
        throw std::runtime_error("Failed to open clipboard");

    std::vector<UINT> formats;

    if (IsClipboardFormatAvailable(CF_DIBV5))
        formats.push_back(CF_DIBV5);

    if (IsClipboardFormatAvailable(CF_DIB))
        formats.push_back(CF_DIB);

    CloseClipboard();
    return formats;
}

bool WindowsClipboardManager::has_image()
{
    return !get_available_image_formats_win32().empty();
}

std::vector<std::string> WindowsClipboardManager::get_available_image_formats()
{
    auto win32_formats = get_available_image_formats_win32();
    if (win32_formats.empty())
        return {};
    return {"png"};
}

std::pair<void*, size_t> WindowsClipboardManager::get_dib()
{
    if (!OpenClipboard(nullptr))
        throw std::runtime_error("Failed to open clipboard");

    HANDLE data_handle = GetClipboardData(CF_DIBV5);
    if (data_handle == nullptr)
        data_handle = GetClipboardData(CF_DIB);

    if (data_handle == nullptr) {
        CloseClipboard();
        return { nullptr, 0 };
    }

    void* original_ptr = GlobalLock(data_handle);
    size_t original_size = GlobalSize(data_handle);

    void* copied_ptr = nullptr;
    if (original_ptr != nullptr && original_size > 0) {
        copied_ptr = GlobalAlloc(GMEM_MOVEABLE, original_size);
        if (copied_ptr != nullptr) {
            void* copy_data = GlobalLock(copied_ptr);
            if (copy_data != nullptr) {
                memcpy(copy_data, original_ptr, original_size);
                GlobalUnlock(copy_data);
            } else {
                GlobalFree(copied_ptr);
                copied_ptr = nullptr;
            }
        }
    }

    if (data_handle != nullptr)
        GlobalUnlock(data_handle);

    CloseClipboard();
    return { copied_ptr, original_size };
}

void WindowsClipboardManager::free_dib(void* dib_data)
{
    if (dib_data != nullptr)
        GlobalFree(dib_data);
}

static int get_encoder_clsid(const WCHAR* format, CLSID* p_clsid)
{
    UINT num = 0, size = 0;
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    std::vector<Gdiplus::ImageCodecInfo> codec_info(size);
    Gdiplus::GetImageEncoders(num, size, codec_info.data());

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(codec_info[j].MimeType, format) == 0) {
            *p_clsid = codec_info[j].Clsid;
            return static_cast<int>(j);
        }
    }
    return -1;
}

// 将 BITMAPV5HEADER 转换为 BITMAPINFO，并返回位数据位置
static const void* convert_dibv5_to_dib(const void* dibv5_data, std::vector<BYTE>& converted_dib)
{
    const BITMAPV5HEADER* v5_header = static_cast<const BITMAPV5HEADER*>(dibv5_data);

    // 计算调色板大小
    int palette_size = 0;
    if (v5_header->bV5BitCount <= 8) {
        palette_size = (1 << v5_header->bV5BitCount) * sizeof(RGBQUAD);
    }

    // 创建 BITMAPINFO 结构
    size_t info_size = sizeof(BITMAPINFOHEADER) + palette_size;
    converted_dib.resize(info_size);

    BITMAPINFO* dst_info = reinterpret_cast<BITMAPINFO*>(converted_dib.data());
    BITMAPINFOHEADER* dst_header = &dst_info->bmiHeader;

    // 复制 BITMAPINFOHEADER 兼容的字段
    dst_header->biSize          = sizeof(BITMAPINFOHEADER);
    dst_header->biWidth         = v5_header->bV5Width;
    dst_header->biHeight        = v5_header->bV5Height;
    dst_header->biPlanes        = v5_header->bV5Planes;
    dst_header->biBitCount      = v5_header->bV5BitCount;
    dst_header->biCompression   = v5_header->bV5Compression;
    dst_header->biSizeImage     = v5_header->bV5SizeImage;
    dst_header->biXPelsPerMeter = v5_header->bV5XPelsPerMeter;
    dst_header->biYPelsPerMeter = v5_header->bV5YPelsPerMeter;
    dst_header->biClrUsed       = v5_header->bV5ClrUsed;
    dst_header->biClrImportant  = v5_header->bV5ClrImportant;

    // 复制调色板（如果存在）
    if (palette_size > 0) {
        const BYTE* src_palette = reinterpret_cast<const BYTE*>(dibv5_data) + sizeof(BITMAPV5HEADER);
        BYTE* dst_palette = reinterpret_cast<BYTE*>(converted_dib.data()) + sizeof(BITMAPINFOHEADER);
        memcpy(dst_palette, src_palette, palette_size);
    }

    // 计算位数据位置
    if (v5_header->bV5BitCount <= 8) {
        int palette_entries = 1 << v5_header->bV5BitCount;
        return reinterpret_cast<const BYTE*>(dibv5_data) + sizeof(BITMAPV5HEADER) + palette_entries * sizeof(RGBQUAD);
    } else {
        return reinterpret_cast<const BYTE*>(dibv5_data) + sizeof(BITMAPV5HEADER);
    }
}

// 从标准 DIB (BITMAPINFO) 获取位数据位置
static const void* get_dib_bits(const BITMAPINFO* dib_info)
{
    const BITMAPINFOHEADER* header = &dib_info->bmiHeader;
    if (header->biBitCount <= 8) {
        int palette_entries = 1 << header->biBitCount;
        return reinterpret_cast<const BYTE*>(dib_info) + header->biSize + palette_entries * sizeof(RGBQUAD);
    } else {
        return reinterpret_cast<const BYTE*>(dib_info) + header->biSize;
    }
}

void WindowsClipboardManager::simulate_paste()
{
    // 一定要确保这两个键此时没有被按下，否则会发生冲突就会吞键
    while(GetAsyncKeyState(VK_CONTROL) & 0x8000) Sleep(10);
    while(GetAsyncKeyState('V') & 0x8000) Sleep(10);

    INPUT inputs[4] = {};

    // 按下 Ctrl
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    // 按下 V
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'V';

    // 松开 V
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'V';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // 松开 Ctrl
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));
}

void WindowsClipboardManager::save_dib_to_png(void* dib_handle, size_t dib_size, const std::wstring& file_path)
{
    if (dib_handle == nullptr || dib_size < sizeof(BITMAPINFOHEADER))
        throw std::invalid_argument("Invalid DIB data");

    if (file_path.empty())
        throw std::invalid_argument("File path cannot be empty");

    ensure_gdiplus_initialized();

    // 锁定 GlobalAlloc 返回的句柄以获取实际指针
    void* dib_data = GlobalLock(dib_handle);
    if (dib_data == nullptr)
        throw std::runtime_error("Failed to lock DIB data");

    // 创建一个 RAII 对象来确保解锁
    struct DIBLock {
        void* handle;
        DIBLock(void* h) : handle(h) {}
        ~DIBLock() { if (handle) GlobalUnlock(handle); }
    } dib_lock(dib_handle);

    // 先检查 header 大小来判断 DIB 格式
    DWORD header_size = *static_cast<DWORD*>(dib_data);

    BITMAPINFO* dib_info_ptr = nullptr;
    const void* bits = nullptr;
    std::vector<BYTE> converted_dib;

    if (header_size == sizeof(BITMAPV5HEADER)) {
        // CF_DIBV5 格式 - 转换为 BITMAPINFO 格式
        bits = convert_dibv5_to_dib(dib_data, converted_dib);
        dib_info_ptr = reinterpret_cast<BITMAPINFO*>(converted_dib.data());
    } else if (header_size == sizeof(BITMAPINFOHEADER)) {
        // CF_DIB 格式 - 直接使用
        dib_info_ptr = static_cast<BITMAPINFO*>(dib_data);
        bits = get_dib_bits(dib_info_ptr);
    } else {
        throw std::invalid_argument("Unsupported DIB format");
    }

    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromBITMAPINFO(dib_info_ptr, const_cast<void*>(bits));

    if (bitmap == nullptr || bitmap->GetLastStatus() != Gdiplus::Ok) {
        delete bitmap;
        throw std::runtime_error("Failed to create bitmap from DIB");
    }

    CLSID png_clsid;
    if (get_encoder_clsid(L"image/png", &png_clsid) == -1) {
        delete bitmap;
        throw std::runtime_error("PNG encoder not found");
    }

    Gdiplus::Status status = bitmap->Save(file_path.c_str(), &png_clsid, nullptr);
    delete bitmap;

    if (status != Gdiplus::Ok)
        throw std::runtime_error("Failed to save PNG file");
}

bool WindowsClipboardManager::save_image_to_png(const std::string& file_path)
{
    auto [dib_data, dib_size] = get_dib();
    if (!dib_data)
        return false;

    try {
        std::wstring wpath = utf8_to_utf16(file_path);
        save_dib_to_png(dib_data, dib_size, wpath);
        free_dib(dib_data);
        return true;
    } catch (std::exception e) {
        free_dib(dib_data);
        throw std::runtime_error(e.what());
    }
}
