cmake_minimum_required(VERSION 3.16)
project(ClipBridge VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(SOURCES
    src/main.cpp
    src/core/actionmanager.cpp
    src/core/appconfig.cpp
    src/ui/trayicon.cpp
    src/ui/settingsdialog.cpp
    src/utils/clipboardhelper.cpp
    src/utils/hotkey.cpp
)

set(HEADERS
    src/core/actionmanager.h
    src/core/appconfig.h
    src/ui/trayicon.h
    src/ui/settingsdialog.h
    src/utils/clipboardhelper.h
    src/utils/hotkey.h
)

add_executable(ClipBridge
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(ClipBridge PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
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
    $<TARGET_FILE_DIR:ClipBridge>/config.json
)
