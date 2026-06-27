# ClipBridge

跨平台快捷键工具，让 Claude TUI / Claude Code 的剪贴板交互更顺畅

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)](#平台支持)
[![Qt](https://img.shields.io/badge/Qt-5.15%2B%20%7C%206.0%2B-green.svg)](#构建指南)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-orange.svg)](CMakeLists.txt)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](CMakeLists.txt)
[![GitHub stars](https://img.shields.io/github/stars/your-username/ClipBridge?style=social)](https://github.com/your-username/ClipBridge)

---

## 目录

[![✨ 核心功能](https://img.shields.io/badge/-核心功能-blue?style=flat-square)](#核心功能)
[![🚀 快速开始](https://img.shields.io/badge/-快速开始-green?style=flat-square)](#快速开始)
[![⚙️ 配置说明](https://img.shields.io/badge/-配置说明-orange?style=flat-square)](#配置说明)
[![🌿 分支说明](https://img.shields.io/badge/-分支说明-purple?style=flat-square)](#分支说明)
[![🛠️ 构建指南](https://img.shields.io/badge/-构建指南-red?style=flat-square)](#构建指南)
[![🖥️ 平台支持](https://img.shields.io/badge/-平台支持-lightgrey?style=flat-square)](#平台支持)
[![📁 项目结构](https://img.shields.io/badge/-项目结构-yellow?style=flat-square)](#项目结构)
[![❓ 常见问题](https://img.shields.io/badge/-常见问题-cyan?style=flat-square)](#常见问题)

---

## 核心功能

### 1. clipboard_image_path —— TUI 发图神器

在 Claude TUI / Claude Code 里发截图，不再需要手动保存文件、复制路径！

- 截图 → 按快捷键 → 自动保存图片 → 自动粘贴路径 → 直接发送
- 支持自定义格式文案（比如 `请查看这张图片：{path}`）
- 支持自定义保存路径

### 2. strip_newlines —— 清理换行符

从终端、日志、文档复制多行文本时，经常因为终端宽度限制被自动插入换行符，导致粘贴后无法直接执行。

- 按快捷键，一键去除所有换行符（`\n` 和 `\r`）
- 命令即贴即用，无需手动清理

### 3. 每个热键独立配置

- 每个快捷键可以设置独立的 `auto_paste` 和 `auto_submit` 行为
- 支持多动作绑定，同一个快捷键按顺序执行多个动作

---

## 快速开始

### 下载运行

1. 从 Releases（未来）下载预编译版本（未来）
2. 或者从源码构建（见下方）
3. 运行程序，系统托盘出现 ClipBridge 图标

### 基本使用

1. 截图 → 按 `Ctrl+Alt+I` → 图片路径自动粘贴到当前输入框
2. 复制多行命令 → 按 `Ctrl+Alt+J` → 换行符已清理，可以直接执行

### 图形界面设置

- 左键/右键点击托盘图标 → 打开设置界面
- 在设置界面里可以：
  - 查看和管理热键
  - 配置图片保存格式和路径
  - 设置默认行为
  - 保存后自动热重载，无需重启

---

## 配置说明

配置文件 `config.json` 在程序同目录下，格式如下：

```json
{
  "hotkeys": [
    {
      "action": "clipboard_image_path",
      "key": "ctrl+Alt+I",
      "behavior": {
        "auto_paste": true,
        "auto_submit": false
      }
    },
    {
      "action": "strip_newlines",
      "key": "ctrl+Alt+J",
      "behavior": {
        "auto_paste": true,
        "auto_submit": false
      }
    }
  ],
  "default_behavior": {
    "auto_paste": true,
    "auto_submit": false
  },
  "output": {
    "format": "{path}",
    "mode": "workspace",
    "dir": ""
  }
}
```

### 字段详解

| 字段 | 类型 | 说明 |
|------|------|------|
| `hotkeys` | array | 快捷键绑定列表 |
| `hotkeys[].action` | string | 动作：`clipboard_image_path` 或 `strip_newlines` |
| `hotkeys[].key` | string | 快捷键，用 `+` 分隔（如 `ctrl+alt+i`） |
| `hotkeys[].behavior` | object | 可选，该热键的独立行为 |
| `default_behavior` | object | 默认行为配置 |
| `default_behavior.auto_paste` | bool | 执行动作后是否自动粘贴 |
| `default_behavior.auto_submit` | bool | 是否自动提交（慎用） |
| `output.format` | string | 粘贴文本格式，`{path}` 会被替换为图片路径 |
| `output.mode` | string | `workspace`（桌面）或 `custom_path`（自定义） |
| `output.dir` | string | `custom_path` 模式下的保存目录 |

---

## 分支说明

> **重要**：两个版本功能一致，区别在于技术栈

| 分支 | 技术栈 | 描述 | 推荐场景 |
|------|--------|------|---------|
| **qt** | Qt 6/5 | 跨平台版本（当前） | 需要 macOS/Linux 支持 |
| **master** | Win32 API | Windows 原生版本 | 只需要 Windows，想要更轻量 |

---

## 构建指南

### 前置要求

- **Qt 5.15.2+** 或 **Qt 6**
- **CMake 3.16+**
- 平台编译器：
  - Windows: Visual Studio 2019+ 或 MinGW
  - macOS: Clang (Xcode Command Line Tools)
  - Linux: GCC/Clang

---

### Windows 构建

1. **下载 Qt**

   访问 <https://www.qt.io/download> 安装 Qt

   - **推荐**：Qt 6.11.1 (LTS)
   - 选择版本：
     - `msvc2019_64` / `msvc2022_64`（配合 Visual Studio）
     - `mingw_64`（配合 MinGW 编译器）

2. **配置 Qt 路径**

   编辑 `CMakeLists.txt`，添加你的 Qt 安装路径：

   ```cmake
   list(APPEND CMAKE_PREFIX_PATH
       # 改为你的实际路径
       "D:/Qt/6.11.1/msvc2019_64"
       "D:/tools/qt/6.11.1/mingw_64"
   )
   ```

3. **构建项目**

   使用 VS Code CMake Tools 插件（推荐），或命令行：

   ```bash
   mkdir build && cd build
   cmake -G "Visual Studio 17 2022" -A x64 ..
   cmake --build . --config Release
   ```

   构建完成后，可执行文件在 `build/Release/` 目录。

---

### macOS 构建

```bash
# 安装 Qt
brew install qt@5

# 构建
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/usr/local/opt/qt@5
cmake --build . --config Release
```

**注意**：首次运行需要在"系统偏好设置 → 安全性与隐私 → 辅助功能"中添加程序。

---

### Linux 构建

```bash
# 安装依赖
sudo apt install qtbase5-dev libx11-dev libxtst-dev

# 构建
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

---

## 平台支持

| 平台 | 热键 | 粘贴 | 说明 |
|------|------|------|------|
| Windows | ✅ | ✅ | 完整支持 |
| Linux X11 | ✅ | ✅ | 需要 X11 |
| macOS | ✅ | ✅ | 需要辅助功能权限 |

---

## 项目结构

```
ClipBridge/
├── CMakeLists.txt           # CMake 构建配置
├── config.json              # 示例配置文件
├── README.md                # 本文档
└── src/
    ├── main.cpp             # 程序入口
    ├── core/
    │   ├── AppConfig.h      # 配置数据结构
    │   ├── AppConfig.cpp    # 配置读写
    │   ├── ActionManager.h  # 动作管理
    │   └── ActionManager.cpp
    ├── ui/
    │   ├── TrayIcon.h       # 托盘图标
    │   ├── TrayIcon.cpp
    │   ├── SettingsDialog.h # 设置对话框
    │   └── SettingsDialog.cpp
    └── utils/
        ├── Hotkey.h         # 全局热键封装
        ├── Hotkey.cpp
        ├── ClipboardHelper.h # 剪贴板工具
        └── ClipboardHelper.cpp
```

---

## 常见问题

### Q: Qt 路径怎么找？

A: Qt 默认安装在：
- Windows: `C:/Qt/` 或 `D:/Qt/`
- macOS: `/usr/local/opt/qt@5`
- Linux: `/usr/include/qt5`

可以在 Qt Creator 的"工具 → 选项 → Kits"里查看。

### Q: 编译错误 "Qt requires a C++17 compiler"

A: 在 CMakeLists.txt 中添加：

```cmake
if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()
```

### Q: 编译警告 "该文件包含不能在当前代码页中表示的字符"

A: 在 CMakeLists.txt 中添加：

```cmake
if(MSVC)
    add_compile_options(/utf-8)
endif()
```

### Q: macOS 上热键不工作

A: 打开"系统偏好设置 → 安全性与隐私 → 辅助功能"，添加并勾选 ClipBridge。

### Q: 想要自定义热键？

A: 两种方式：
1. 编辑 `config.json` 文件
2. 使用图形设置界面（开发中）

---

## 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。

---

## 贡献

欢迎提交 Issue 和 Pull Request！

---

Made with ❤️ for Claude TUI users
