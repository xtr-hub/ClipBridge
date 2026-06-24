cmake_minimum_required(VERSION 3.16)
project(ClipBridge VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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
    src/utils/ClipboardHelper.cpp
    src/utils/Hotkey.cpp
)

set(HEADERS
    src/core/ActionManager.h
    src/core/AppConfig.h
    src/ui/TrayIcon.h
    src/ui/SettingsDialog.h
    src/utils/ClipboardHelper.h
    src/utils/Hotkey.h
)

add_executable(ClipBridge
    ${SOURCES}
    ${HEADERS}
)

# Link Qt libraries (works for both Qt5 and Qt6)
target_link_libraries(ClipBridge PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(WIN32)
    set_target_properties(ClipBridge PROPERTIES
        WIN32_EXECUTABLE ON
    )
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
    $<TARGET_FILE_DIR>/config.json
)
