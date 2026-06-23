# ClipBridge

ClipBridge 是一个 Windows 快捷键工具，专为解决 Claude TUI / Claude Code 等终端环境下的剪贴板交互痛点而设计。

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

按下 `Ctrl+Alt+J` 后，自动移除剪贴板文本中的所有换行符（`\n` 和 `\r`），让命令即贴即用。

### 多动作绑定

同一个快捷键可以绑定多个动作，按列表顺序依次执行。

## 配置文件

项目使用 JSON 配置。当前示例格式如下：

```json
{
  "hotkeys": [
    {
      "action": "clipboard_image_path",
      "key": "ctrl+Alt+I"
    },
    {
      "action": "strip_newlines",
      "key": "ctrl+Alt+J"
    }
  ],
  "behavior": {
    "auto_paste": true,
    "auto_submit": false
  },
  "output": {
    "format": "请查看这张剪贴板图片分析内容\n{path}",
    "path": {
      "mode": "custom_path",
      "dir": "C:/Users/37863/Desktop/临时图片"
    }
  }
}
```

字段说明：

- `hotkeys`: 快捷键绑定列表。顺序决定动作执行顺序。
- `hotkeys[].action`: 快捷键触发的动作，可用 `clipboard_image_path` 或 `strip_newlines`。
- `hotkeys[].key`: 触发快捷键，使用 `+` 分隔按键。
- `behavior.auto_paste`: 生成路径后是否自动粘贴到当前窗口。
- `behavior.auto_submit`: 是否自动提交，默认建议保持 `false`。
- `output.format`: 粘贴文本格式，`{path}` 会被替换为图片路径。
- `output.path.mode`: 图片保存路径模式，支持 `custom_path` 和 `workspace`。
- `output.path.dir`: `custom_path` 模式下的图片保存目录。

## 构建

### 前置要求

- Qt 6.2+
- CMake 3.16+
- Visual Studio 2019+ (Windows)

### 构建步骤

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6
cmake --build . --config Release
```

### Visual Studio 构建示例

```bash
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/msvc2019_64 ..
cmake --build . --config Release
```

## 项目结构

```
src/
├── main.cpp                 # 程序入口
├── core/
│   ├── appconfig.h/cpp      # 配置管理
│   └── actionmanager.h/cpp  # 动作执行
├── ui/
│   └── trayicon.h/cpp       # 托盘图标
└── utils/
    ├── clipboardhelper.h/cpp # 剪贴板工具
    └── hotkey.h/cpp         # 全局热键
```
