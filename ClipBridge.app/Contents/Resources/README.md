<div align="center">
  <img src="resources/icon.png" alt="logo" width="200" height="200">

  # ClipBridge

  跨平台快捷键工具，让 Claude TUI / Claude Code 的剪贴板交互更顺畅

  <div style="display: flex; justify-content: center; gap: 12px; margin-bottom: 12px; flex-wrap: wrap;">
    <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-blue.svg?style=flat&logo=github" alt="License"></a>
    <a href="#平台支持"><img src="https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg?style=flat" alt="Platform"></a>
    <a href="https://github.com/xtr-hub/ClipBridge/releases"><img src="https://img.shields.io/badge/Download-Releases-green.svg?style=flat&logo=github" alt="Releases"></a>
  </div>
</div>

[中文](README.md) | [English](README_EN.md)

## 能干什么

- TUI 发图神器：截图 → 按快捷键 → 自动保存图片 → 自动粘贴路径 → 直接发送
- 一键复制文件路径：复制文件 → 按快捷键 → 自动粘贴文件路径列表
- 一键去除换行符：从终端复制多行命令，自动清理换行，即贴即用
- 自定义输出格式：支持 `{path}` 占位符，每个动作可配置不同文案
- 图形化热键管理：添加/编辑/删除热键，每个热键独立配置自动粘贴和自动提交
- 自动热重载：保存配置后立即生效，无需重启
- 跨平台支持：Windows / macOS / Linux

## 快速开始

### 下载运行

1. 从 [Releases](https://github.com/xtr-hub/ClipBridge/releases) 下载预编译版本
2. 或者从源码构建（见下方）
3. 运行程序，系统托盘出现 ClipBridge 图标

### 基本使用

1. 截图 → 按 `Ctrl+Alt+I` → 图片路径自动粘贴到当前输入框
2. 复制文件 → 按 `Ctrl+Alt+F` → 文件路径自动粘贴到当前输入框
3. 复制多行命令 → 按 `Ctrl+Alt+J` → 换行符已清理，可以直接执行

### 图形界面设置

**左键/右键点击托盘图标** → 打开设置界面

**设置界面功能：**

1. **快捷键管理**
   - 查看现有热键列表（显示动作、按键、行为配置）
   - 添加新热键
   - 编辑现有热键（修改动作、按键、行为配置）
   - 删除热键

2. **输出设置**
   - 配置全局默认输出格式（支持 `{path}` 占位符）
   - 为每个动作配置专属输出格式，未配置时自动回退到全局格式
   - 选择保存模式：桌面默认路径 / 自定义路径
   - 图形化选择自定义保存目录

3. **默认行为配置**
   - 配置新添加热键的默认行为
   - 自动粘贴
   - 自动提交

4. **保存后自动热重载**
   - 无需重启程序，热键配置立即生效

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
    },
    {
      "action": "clipboard_file_path",
      "key": "ctrl+Alt+F",
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
    "formats": {
      "clipboard_image_path": "请查看这张剪贴板图片分析内容\n{path}",
      "clipboard_file_path": "请查看这个文件\n{path}"
    },
    "mode": "workspace",
    "dir": ""
  }
}
```

### 字段详解

| 字段 | 类型 | 说明 |
|------|------|------|
| `hotkeys` | array | 快捷键绑定列表 |
| `hotkeys[].action` | string | 动作：`clipboard_image_path`、`clipboard_file_path` 或 `strip_newlines` |
| `hotkeys[].key` | string | 快捷键，用 `+` 分隔（如 `ctrl+alt+i`） |
| `hotkeys[].behavior` | object | 可选，该热键的独立行为 |
| `default_behavior` | object | 默认行为配置 |
| `default_behavior.auto_paste` | bool | 执行动作后是否自动粘贴 |
| `default_behavior.auto_submit` | bool | 是否自动提交（慎用） |
| `output.format` | string | 全局默认输出格式，`{path}` 会被替换为路径 |
| `output.formats` | object | 按动作的专属输出格式，未配置时回退到 `output.format` |
| `output.mode` | string | `workspace`（桌面）或 `custom_path`（自定义） |
| `output.dir` | string | `custom_path` 模式下的保存目录 |

---

## 分支说明

> **重要**：两个版本功能一致，区别在于技术栈

| 分支 | 技术栈 | 描述 | 推荐场景 |
|------|--------|------|---------|
| **qt** | Qt 6/5 | 跨平台版本（当前） | 需要 macOS/Linux 支持 |
| **win32** | Win32 API | Windows 原生版本 | 只需要 Windows，想要更轻量 |

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

   编辑 `CMakeLists.txt`，添加你的 Qt 安装路径（默认已配置常见路径）：

   ```cmake
   list(APPEND CMAKE_PREFIX_PATH
       # 改为你的实际路径
       "C:/Qt/6.11.1/msvc2019_64"
       "D:/tools/qt/6.11.1/mingw_64"
   )
   ```

3. **构建项目**

   方式一：使用 MinGW（推荐）
   ```bash
   cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j8
   ```

   方式二：使用 Visual Studio
   ```bash
   mkdir build && cd build
   cmake -G "Visual Studio 17 2022" -A x64 ..
   cmake --build . --config Release
   ```

   构建完成后，可执行文件在 `build/`（MinGW）或 `build/Release/`（Visual Studio）目录。
   CMake 会自动运行 `windeployqt` 复制 Qt DLL 到输出目录。

4. **一键构建打包（推荐）**

   项目包含一键构建打包脚本，一行命令搞定：
   ```cmd
   # 使用批处理脚本
   scripts/package/build_and_package.bat

   # 或使用 PowerShell 脚本
   powershell -ExecutionPolicy Bypass -File scripts/package/build_and_package.ps1
   ```

5. **打包发布**

   如果你已经手动构建过，只想打包，运行：
   ```cmd
   # 使用批处理脚本
   scripts/package/package.bat

   # 或使用 PowerShell 脚本
   powershell -ExecutionPolicy Bypass -File scripts/package/package.ps1
   ```

   脚本会创建包含所有依赖的 `ClipBridge_Qt` 文件夹和 zip 压缩包。

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

**打包**

```bash
./scripts/package/package_macos.sh
```

脚本会创建 `ClipBridge.app` 和 `ClipBridge_macOS_<版本>.zip`。

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

详见 [项目结构](docs/STRUCTURE.md)。

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
1. **推荐：使用图形设置界面（点击托盘图标 → 打开设置）
2. 手动编辑 `config.json` 文件

---

## 贡献

欢迎提交 Issue 和 Pull Request！

贡献前请阅读 [贡献指南](docs/CONTRIBUTING.md)。

感谢所有为 ClipBridge 做出贡献的人！

---

## 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。
