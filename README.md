# ClipBridge (Windows 原生版)

> 注意：此分支仍存在许多未完成的功能

Windows 快捷键工具，让 Claude TUI / Claude Code 的剪贴板交互更顺畅

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://github.com/xtr-hub/ClipBridge)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](CMakeLists.txt)
[![CMake](https://img.shields.io/badge/CMake-3.10%2B-orange.svg)](CMakeLists.txt)
[![GitHub stars](https://img.shields.io/github/stars/xtr-hub/ClipBridge?style=social)](https://github.com/xtr-hub/ClipBridge)

---

## 版本说明

当前版本：**v1.0.0**

### v1.0.0 更新内容
- 统一 Windows / macOS 版本号到 1.0.0
- 修复 win32 版设置对话框关闭按钮问题
- 优化 win32 版对话框宽度
- 统一压缩包命名格式：`ClipBridge_Windows_<version>.zip`
- 自动从 CMakeLists.txt 读取版本号

---

## 目录

[![✨ 核心功能](https://img.shields.io/badge/-核心功能-blue?style=flat-square)](#核心功能)
[![🚀 快速开始](https://img.shields.io/badge/-快速开始-green?style=flat-square)](#快速开始)
[![⚙️ 配置说明](https://img.shields.io/badge/-配置说明-orange?style=flat-square)](#配置说明)
[![🌿 分支说明](https://img.shields.io/badge/-分支说明-purple?style=flat-square)](#分支说明)
[![🛠️ 构建指南](https://img.shields.io/badge/-构建指南-red?style=flat-square)](#构建指南)
[![📁 项目结构](https://img.shields.io/badge/-项目结构-yellow?style=flat-square)](#项目结构)
[![❓ 常见问题](https://img.shields.io/badge/-常见问题-cyan?style=flat-square)](#常见问题)

---

## 分支说明

> **重要**：两个版本功能一致，区别在于技术栈

| 分支 | 技术栈 | 描述 | 推荐场景 |
| --- | --- | --- | --- |
| **win32** | Win32 API | Windows 原生版本（当前） | 只需要 Windows，轻量无依赖 |
| **qt** | Qt 6/5 | Qt 跨平台版本 | 需要 macOS/Linux 支持 |

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

1. 从 Releases（未来）下载预编译版本
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
| --- | --- | --- |
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

## 构建指南

### 前置要求

- **CMake 3.10+**
- **Visual Studio 2019+**
- vcpkg（可选，用于 nlohmann_json）

### 一键构建打包（推荐）

项目包含一键构建打包脚本，一行命令搞定：
```cmd
# 使用批处理脚本
build_and_package.bat

# 或使用 PowerShell 脚本
powershell -ExecutionPolicy Bypass -File build_and_package.ps1
```

**注意**：打包脚本会自动从 `CMakeLists.txt` 读取版本号，生成 `ClipBridge_Windows_<version>.zip` 压缩包。

### 构建步骤

如果你想手动构建：
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

构建完成后，可执行文件在 `build/Release/` 目录。

### 打包发布

如果你已经手动构建过，只想打包，运行：
```cmd
# 使用批处理脚本
package.bat

# 或使用 PowerShell 脚本
powershell -ExecutionPolicy Bypass -File package.ps1
```

脚本会创建包含所有依赖的 `ClipBridge_Win32` 文件夹和 `ClipBridge_Windows_<version>.zip` 压缩包。

### 使用 vcpkg

如果你想用 vcpkg 安装依赖，在项目根目录：

```bash
vcpkg install nlohmann-json:x64-windows
```

然后正常 CMake 构建即可。

---

## 项目结构

```text
ClipBridge/
├── CMakeLists.txt           # CMake 构建配置
├── config.json              # 示例配置文件
├── README.md                # 本文档
├── include/
│   ├── ClipBridge.h         # 主程序头文件
│   ├── framework.h
│   ├── Resource.h
│   ├── targetver.h
│   └── core/
│       ├── app_config.hpp   # 配置数据结构
│       ├── action_manager.hpp # 动作管理
│       ├── config_manager.hpp # 配置读写
│       ├── hotkey_keys.hpp  # 热键映射
│       ├── clipboard_manager.hpp # 剪贴板管理
│       ├── register_manager.hpp # 热键注册
│       ├── path_processer.hpp # 路径处理
│       └── string_utils.hpp # 字符串工具
├── src/
│   ├── ClipBridge.cpp       # 主程序入口（WinMain）
│   └── core/
│       ├── action_manager.cpp
│       ├── config_manager.cpp
│       ├── hotkey_keys.cpp
│       ├── clipboard_manager.cpp
│       ├── register_manager.cpp
│       ├── path_processer.cpp
│       └── string_utils.cpp
├── resources/
│   ├── ClipBridge.rc        # Windows 资源文件
│   ├── ClipBridge.ico       # 程序图标
│   └── small.ico
└── test/                    # 测试代码
```

---

## 常见问题

### Q: 编译警告 "该文件包含不能在当前代码页中表示的字符"

A: 在 CMakeLists.txt 中已经添加了 `/utf-8` 编译选项，应该没问题。如果还有问题，确保文件保存为 UTF-8 with BOM 格式。

### Q: 找不到 nlohmann/json.hpp？

A: 项目支持两种方式：

1. 使用 vcpkg 安装：`vcpkg install nlohmann-json:x64-windows`
2. 使用头文件-only 模式：项目会自动检测并使用

### Q: 想要自定义热键？

A: 两种方式：

1. 编辑 `config.json` 文件
2. 使用图形设置界面

---

## 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。

---

## 贡献

欢迎提交 Issue 和 Pull Request！

---

Made with ❤️ for Claude TUI users
