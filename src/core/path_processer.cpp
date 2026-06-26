#include "core/path_processer.hpp"
#include <windows.h>
#include <shlobj.h>
#include <stdexcept>

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

std::string PathProcesser::get_workspace_path()
{
    wchar_t docs_path[MAX_PATH] = {0};
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, 0, docs_path))) {
        std::wstring result = docs_path;
        result += L"\\ClipBridge";
        int size = WideCharToMultiByte(CP_UTF8, 0, result.c_str(), (int)result.size(), nullptr, 0, nullptr, nullptr);
        if (size <= 0) return default_path("ClipBridge");
        std::string utf8_str(size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, result.c_str(), (int)result.size(), &utf8_str[0], size, nullptr, nullptr);
        return utf8_str;
    }
    return "ClipBridge";
}

std::string PathProcesser::default_path(const std::string& dir_name)
{
    wchar_t temp_path[MAX_PATH] = { 0 };
    DWORD result = GetTempPathW(MAX_PATH, temp_path);
    if (result == 0 || result > MAX_PATH) throw std::runtime_error("Failed to read tempdir");

    std::wstring full_path = temp_path;
    full_path += utf8_to_utf16(dir_name);

    int size = WideCharToMultiByte(CP_UTF8, 0, full_path.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) throw std::runtime_error("Failed to convert utf-8");
    std::string utf8_str(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, full_path.c_str(), -1, &utf8_str[0], size, nullptr, nullptr);
    utf8_str = normalize_path(utf8_str);
    return utf8_str;
}

std::string PathProcesser::normalize_path(const std::string& path)
{
    std::string result = path;
    for (char& ch : result) {
        if (ch == '/')
            ch = '\\';
    }
    return result;
}

std::string PathProcesser::join_paths(const std::string& base, const std::string& relative)
{
    if (base.empty())
        return relative;

    std::string result = base;
    if (!result.empty() && result.back() != '\\')
        result += '\\';
    result += relative;
    return result;
}

bool PathProcesser::ensure_directory_exists(const std::string& path)
{
    std::wstring wpath = utf8_to_utf16(path);

    DWORD attrs = GetFileAttributesW(wpath.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }

    std::wstring current;
    std::wstring remaining = wpath;

    while (!remaining.empty()) {
        size_t pos = remaining.find_first_of(L"\\/");
        std::wstring part;
        if (pos != std::wstring::npos) {
            part = remaining.substr(0, pos);
            remaining = remaining.substr(pos + 1);
        } else {
            part = remaining;
            remaining.clear();
        }

        if (!part.empty()) {
            if (!current.empty() && current.back() != L'\\')
                current += L'\\';
            current += part;

            attrs = GetFileAttributesW(current.c_str());
            if (attrs == INVALID_FILE_ATTRIBUTES) {
                if (!CreateDirectoryW(current.c_str(), nullptr)) {
                    if (GetLastError() != ERROR_ALREADY_EXISTS)
                        return false;
                }
            } else if (!(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                return false;
            }
        }
    }
    return true;
}

std::string PathProcesser::get_config_directory()
{
    wchar_t appdata_path[MAX_PATH] = {0};
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appdata_path))) {
        std::wstring result = appdata_path;
        result += L"\\ClipBridge";
        int size = WideCharToMultiByte(CP_UTF8, 0, result.c_str(), (int)result.size(), nullptr, 0, nullptr, nullptr);
        if (size <= 0) return default_path("config");
        std::string utf8_str(size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, result.c_str(), (int)result.size(), &utf8_str[0], size, nullptr, nullptr);
        return utf8_str;
    }
    return default_path("config");
}
