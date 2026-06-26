# CMakeList.txt: ClipBridge 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)

# 设置 vcpkg 工具链（如果存在） - 必须在 project() 之前设置
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/share/vcpkg-cmake/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/share/vcpkg-cmake/vcpkg.cmake")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/scripts/buildsystems/vcpkg.cmake")
elseif(EXISTS "D:/tools/vcpkg/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "D:/tools/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ClipBridge")

# 添加源文件（不包含 ClipBridge.cpp，因为它有 WinMain）
set(SOURCES
    "src/core/action_manager.cpp"
    "src/core/config_manager.cpp"
    "src/core/hotkey_keys.cpp"
    "src/core/clipboard_manager.cpp"
    "src/core/register_manager.cpp"
    "src/core/path_processer.cpp"
    "src/core/string_utils.cpp"
    "src/ui/settings_dialog.cpp"
)

# 添加头文件
set(HEADERS
    "include/ClipBridge.h"
    "include/framework.h"
    "include/targetver.h"
    "include/Resource.h"
    "include/core/app_config.hpp"
    "include/core/action_manager.hpp"
    "include/core/config_manager.hpp"
    "include/core/hotkey_keys.hpp"
    "include/core/clipboard_manager.hpp"
    "include/core/register_manager.hpp"
    "include/core/path_processer.hpp"
    "include/core/string_utils.hpp"
    "include/ui/settings_dialog.hpp"
)

# 添加资源文件
set(RESOURCES
    "resources/ClipBridge.rc"
)

# 创建核心库（不含 WinMain，供测试使用）
add_library(ClipBridgeCore STATIC
    ${SOURCES}
    ${HEADERS}
)

# 将源代码添加到此项目的可执行文件（包含 WinMain）
add_executable (ClipBridge WIN32
    "src/ClipBridge.cpp"
    ${RESOURCES}
)

# 包含头文件目录（核心库）
target_include_directories(ClipBridgeCore PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
)

# 包含头文件目录（主程序）
target_include_directories(ClipBridge PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
)

# 尝试查找 nlohmann_json，如果找不到就使用头文件-only 方式
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
  target_link_libraries(ClipBridgeCore PUBLIC nlohmann_json::nlohmann_json)
else()
  message(STATUS "nlohmann_json not found, using header-only mode")
endif()

# 链接 GDI+ 库
target_link_libraries(ClipBridgeCore PUBLIC gdiplus)

# 主程序链接核心库
target_link_libraries(ClipBridge PRIVATE ClipBridgeCore)

# 设置 Windows 子系统
if(MSVC)
  target_compile_definitions(ClipBridge PRIVATE _UNICODE UNICODE)
  target_compile_definitions(ClipBridgeCore PRIVATE _UNICODE UNICODE)
  target_compile_options(ClipBridge PRIVATE /utf-8)
  target_compile_options(ClipBridgeCore PRIVATE /utf-8)
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ClipBridge PROPERTY CXX_STANDARD 20)
  set_property(TARGET ClipBridgeCore PROPERTY CXX_STANDARD 20)
endif()

# 复制 config.json 到输出目录
add_custom_command(TARGET ClipBridge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
    $<TARGET_FILE_DIR:ClipBridge>/config.json
)

# 添加测试
# enable_testing()
add_subdirectory(test)
