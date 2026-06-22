# CMakeList.txt: ClipBridge 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ClipBridge")

# 设置 vcpkg 工具链（如果存在）
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/share/vcpkg-cmake/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/share/vcpkg-cmake/vcpkg.cmake")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/scripts/buildsystems/vcpkg.cmake")
endif()

# 添加本地 vcpkg include 目录
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/include")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/include")
endif()

# 添加源文件
set(SOURCES
    "源文件/ClipBridge.cpp"
    "源文件/core/action_manager.cpp"
    "源文件/core/config_manager.cpp"
    "源文件/core/hotkey_keys.cpp"
    "源文件/platform/windows/windows_clipboard_manager.cpp"
    "源文件/platform/windows/windows_register_manager.cpp"
    "源文件/platform/windows/windows_path_processer.cpp"
    "源文件/platform/platform_factory.cpp"
)

# 添加头文件
set(HEADERS
    "头文件/ClipBridge.h"
    "头文件/framework.h"
    "头文件/targetver.h"
    "头文件/Resource.h"
    "头文件/core/app_config.hpp"
    "头文件/core/action_manager.hpp"
    "头文件/core/config_manager.hpp"
    "头文件/core/hotkey_keys.hpp"
    "头文件/platform/clipboard_manager.hpp"
    "头文件/platform/register_manager.hpp"
    "头文件/platform/path_processer.hpp"
    "头文件/platform/windows/windows_clipboard_manager.hpp"
    "头文件/platform/windows/windows_register_manager.hpp"
    "头文件/platform/windows/windows_path_processer.hpp"
)

# 添加资源文件
set(RESOURCES
    "资源文件/ClipBridge.rc"
)

# 将源代码添加到此项目的可执行文件。
add_executable (ClipBridge WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

# 包含头文件目录
target_include_directories(ClipBridge PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/头文件"
    "${CMAKE_CURRENT_SOURCE_DIR}/头文件/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/头文件/platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/头文件/platform/windows"
    "${CMAKE_CURRENT_SOURCE_DIR}/资源文件"
)

# 尝试查找 nlohmann-json，如果找不到就使用头文件-only 方式
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
  target_link_libraries(ClipBridge PRIVATE nlohmann_json::nlohmann_json)
else()
  message(STATUS "nlohmann_json not found, using header-only mode")
endif()

# 链接 GDI+ 库
target_link_libraries(ClipBridge PRIVATE gdiplus)

# 设置 Windows 子系统
if(MSVC)
  target_compile_definitions(ClipBridge PRIVATE _UNICODE UNICODE)
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ClipBridge PROPERTY CXX_STANDARD 20)
endif()

# 复制 config.json 到输出目录
add_custom_command(TARGET ClipBridge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
    $<TARGET_FILE_DIR:ClipBridge>/config.json
)
