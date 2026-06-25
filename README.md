# ClipBridge (Windows 原生版)

ClipBridge 是一个 Windows 快捷键工具，专为解决 Claude TUI / Claude Code 等终端环境下的剪贴板交互痛点而设计。

> ⚠️ 注意：这是 Windows 原生版本（无 Qt 依赖）。如果你需要跨平台支持（macOS/Linux），请切换到 `qt` 分支。

## 分支说明

| 分支 | 描述 | 推荐场景 |
|------|------|---------|
| **master** | Windows 原生版本（当前） | 只需要 Windows，轻量无依赖 |
| **qt** | Qt 跨平台版本 | 需要 macOS/Linux 支持 |

## 核心价值

### 解决的痛点

在使用终端工具时，以下两个场景特别让人头疼：

1. **截图无法直接粘贴** —— 网页聊天界面里随手一贴就能发图，但终端里不行
2. **复制命令带多余换行** —— 从终端复制多行命令时，因为终端自动换行导致粘贴后无法直接执行

ClipBridge 用两个快捷键解决这两个问题。

### clipboard_image_path —— TUI 里的图片引用

按下快捷键后，自动把剪贴板里的图片保存为本地 PNG 文件，然后把文件路径（配合自定义格式文案）填入剪贴板并自动粘贴。

这样你在 Claude TUI 里想发截图时：

- 截图 → 按下 `Ctrl+Alt+I` → 图片路径自动出现在输入框里 → 直接发送
- Claude Code 可以读取本地路径的图片，体验就和网页版一样顺畅

### strip_newlines —— 一键去除多余换行

从终端、日志、文档里复制多行文本或命令时，终端渲染引擎经常会因为宽度限制自动插入换行符，导致粘贴后不能直接执行。

按下 `Ctrl+Alt+J` 后，自动移除剪贴板文本里的所有换行符（`\n` 和 `\r`），让命令即贴即用。

### 多动作绑定

同一个快捷键可以绑定多个动作，按列表顺序依次执行。

## 配置文件

项目使用 JSON 配置。当前示例格式如下：

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

### 多动作绑定示例

```json
{
  "hotkeys": [
    {
      "action": "strip_newlines",
      "key": "ctrl+Alt+K"
    },
    {
      "action": "clipboard_image_path",
      "key": "ctrl+Alt+K"
    }
  ]
}
```

按下 `Ctrl+Alt+K` 时，会先执行 `strip_newlines`，再执行 `clipboard_image_path`。

字段说明：

- `hotkeys`: 快捷键绑定列表。
- `hotkeys[].action`: 快捷键触发的动作，可用 `clipboard_image_path` 或 `strip_newlines`。
- `hotkeys[].key`: 触发快捷键，使用 `+` 分隔按键。
- `hotkeys[].behavior`: (可选) 该热键的专属行为配置，不填则使用 `default_behavior`。
- `default_behavior`: 默认行为配置，用于没有单独配置 `behavior` 的热键。
- `default_behavior.auto_paste`: 执行动作后是否自动粘贴到当前窗口。
- `default_behavior.auto_submit`: 是否自动提交，默认建议保持 `false`。
- `output.format`: 粘贴文本格式，`{path}` 会被替换为图片路径。
- `output.mode`: 图片保存路径模式，支持 `custom_path` 和 `workspace`（保存到桌面的 ClipBridge Images 目录）。
- `output.dir`: `custom_path` 模式下的图片保存目录。

项目仍在开发中，功能和配置格式可能会继续变化。

## 构建 (Windows 原生版)

### 前置要求

- CMake 3.10+
- Visual Studio 2019+
- vcpkg (可选，用于 nlohmann_json)

### 构建步骤

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### 使用 vcpkg

如果你想用 vcpkg 安装依赖，在项目根目录：

```bash
vcpkg install nlohmann-json:x64-windows
```

然后正常 CMake 构建即可。
