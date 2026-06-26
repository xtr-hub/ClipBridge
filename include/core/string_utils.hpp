#pragma once

#include <string>

namespace string_utils {

/**
 * @brief Convert UTF-8 string to UTF-16 wide string
 * @param str UTF-8 encoded string
 * @return UTF-16 encoded wide string
 */
std::wstring utf8_to_wide(const std::string& str);

/**
 * @brief Convert UTF-16 wide string to UTF-8 string
 * @param wstr UTF-16 encoded wide string
 * @return UTF-8 encoded string
 */
std::string wide_to_utf8(const std::wstring& wstr);

} // namespace string_utils
