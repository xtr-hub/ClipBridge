cmake_minimum_required(VERSION 3.16)
project(ClipBridge VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# MSVC 编译选项
if(MSVC)
    add_compile_options(/Zc:__cplusplus)
    add_compile_options(/utf-8)
    add_compile_options(/permissive-)
endif()

# Common Qt installation paths (for convenience)
# Prefer environment variables, fall back to default paths
if(NOT DEFINED ENV{CMAKE_PREFIX_PATH} AND NOT DEFINED ENV{QTDIR} AND NOT DEFINED ENV{Qt6_DIR} AND NOT DEFINED ENV{Qt5_DIR})
    list(APPEND CMAKE_PREFIX_PATH
        # macOS
        "~/Qt/5.15.2/clang_64"
        "~/5.15.2/clang_64"
        "/usr/local/opt/qt5"
        "/opt/homebrew/opt/qt5"
        "/opt/homebrew/opt/qt6"
        # Windows
        "C:/Qt/5.15.2/msvc2019_64"
        "C:/Qt/6.5.0/msvc2019_64"
        "D:/tools/qt/6.11.1/mingw_64"
        # Linux
        "/usr"
        "/opt/qt5"
    )
else()
    # Use environment variables if set
    if(DEFINED ENV{QTDIR})
        list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")
    endif()
    if(DEFINED ENV{CMAKE_PREFIX_PATH})
        list(APPEND CMAKE_PREFIX_PATH "$ENV{CMAKE_PREFIX_PATH}")
    endif()
endif()

# Try to find Qt6 first, then Qt5
find_package(Qt6 QUIET COMPONENTS Core Gui Widgets)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Building with Qt6")
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Building with Qt5")
endif()

set(SOURCES
    src/main.cpp
    src/core/ActionManager.cpp
    src/core/AppConfig.cpp
    src/ui/TrayIcon.cpp
    src/ui/SettingsDialog.cpp
    src/ui/HotkeyEditDialog.cpp
    src/utils/ClipboardHelper.cpp
    src/utils/Hotkey.cpp
    src/utils/Simulator.cpp
)

set(HEADERS
    src/core/ActionManager.h
    src/core/AppConfig.h
    src/ui/TrayIcon.h
    src/ui/SettingsDialog.h
    src/ui/HotkeyEditDialog.h
    src/utils/ClipboardHelper.h
    src/utils/Hotkey.h
    src/utils/Simulator.h
)

add_executable(ClipBridge
    ${SOURCES}
    ${HEADERS}
    resources.qrc
)

# Link Qt libraries (works for both Qt5 and Qt6)
target_link_libraries(ClipBridge PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(WIN32)
    # 不在 CMake 里设置 WIN32_EXECUTABLE，避免 MinGW 链接问题
    # 在 main.cpp 里用 FreeConsole() 隐藏控制台窗口
elseif(APPLE)
    find_library(APP_SERVICES ApplicationServices)
    target_link_libraries(ClipBridge PRIVATE ${APP_SERVICES})
elseif(UNIX AND NOT APPLE)
    find_package(X11 QUIET)
    if(X11_FOUND)
        find_package(X11_XTest QUIET)
        if(X11_XTest_FOUND)
            target_link_libraries(ClipBridge PRIVATE X11::X11 X11::XTest)
        endif()
    endif()
endif()

target_include_directories(ClipBridge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)

add_custom_command(TARGET ClipBridge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
    $<TARGET_FILE_DIR:ClipBridge>/config.json
)

# Copy Qt DLLs on Windows - using windeployqt instead for reliability
if(WIN32)
    # Find windeployqt
    if(QT_VERSION_MAJOR EQUAL 6)
        get_target_property(_qt_core_location Qt6::Core LOCATION)
    else()
        get_target_property(_qt_core_location Qt5::Core LOCATION)
    endif()
    get_filename_component(_qt_bin_dir "${_qt_core_location}" DIRECTORY)
    find_program(_windeployqt_exe windeployqt PATHS "${_qt_bin_dir}")

    if(_windeployqt_exe)
        # Run windeployqt to copy all required Qt DLLs and plugins
        add_custom_command(TARGET ClipBridge POST_BUILD
            COMMAND "${_windeployqt_exe}"
            --no-translations
            --no-system-d3d-compiler
            --no-opengl-sw
            --no-quick-import
            "$<TARGET_FILE:ClipBridge>"
            COMMENT "Deploying Qt ${QT_VERSION_MAJOR} runtime using windeployqt"
        )
    else()
        message(WARNING "windeployqt not found - you'll need to manually copy Qt DLLs")
    endif()
endif()
