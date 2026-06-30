#include "core/string_utils.hpp"
#include <windows.h>

namespace string_utils {


std::string to_lower(std::string value)
    {
        for (char& ch : value)
        {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        return value;
    }

std::vector<std::string> split_string(const std::string& str, char separator)
{
        std::vector<std::string> result;
        std::string current;

        for (char ch : str)
        {
            if (ch == separator)
            {
                if (!current.empty())
                {
                    result.push_back(current);
                    current.clear();
                }
            }
            else if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
            {
                current.push_back(ch);
            }
        }

        if (!current.empty())
        {
            result.push_back(current);
        }

        return result;
}

std::wstring utf8_to_wide(const std::string& str)
{
    if (str.empty()) return {};
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    if (size <= 0) return {};
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], size);
    return result;
}

std::string wide_to_utf8(const std::wstring& wstr)
{
    if (wstr.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    if (size <= 0) return {};
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], size, nullptr, nullptr);
    return result;
}

} // namespace string_utils
