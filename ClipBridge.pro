QT += core gui widgets

CONFIG += c++17

TARGET = ClipBridge
TEMPLATE = app

# 定义源文件
SOURCES += \
    src/main.cpp \
    src/core/ActionManager.cpp \
    src/core/AppConfig.cpp \
    src/ui/TrayIcon.cpp \
    src/ui/SettingsDialog.cpp \
    src/utils/ClipboardHelper.cpp \
    src/utils/Hotkey.cpp

# 定义头文件
HEADERS += \
    src/core/ActionManager.h \
    src/core/AppConfig.h \
    src/ui/TrayIcon.h \
    src/ui/SettingsDialog.h \
    src/utils/ClipboardHelper.h \
    src/utils/Hotkey.h

# Windows 平台特定设置
win32 {
    # Windows 下不显示控制台窗口
    CONFIG += windows
}

# macOS 平台特定设置
macx {
    # 链接 ApplicationServices 框架
    LIBS += -framework ApplicationServices
}

# Linux 平台特定设置
unix:!macx {
    # X11 支持
    QT += x11extras
    LIBS += -lX11 -lXtst
}

# 安装配置文件到输出目录
CONFIG += file_copies
COPIES += config
config.files = config.json
config.path = $$OUT_PWD

# 包含目录
INCLUDEPATH += \
    src \
    src/core \
    src/ui \
    src/utils
