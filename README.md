# ClipVision

ClipVision 是一个 Windows 快捷键工具，用于把剪贴板图片快速保存为本地文件，并将文件路径粘贴到当前输入窗口。

在使用 Claude TUI / Claude Code 这类终端工具时，截图和剪贴板图片通常不能像聊天界面那样直接粘贴使用。ClipVision 试图解决这个痛点：按下快捷键后，它会把剪贴板中的图片落盘，并把图片路径送到当前输入位置，让终端里的图片引用更顺手。

## 配置文件

项目使用 JSON 配置。当前示例格式如下：

```json
{
  "hotkeys": [
    {
      "action": "clipboard_image_path",
    "key": "ctrl+Alt+I"
    }
  ],
  "behavior": {
    "auto_paste": true,
    "auto_submit": false
  },
  "output": {
    "mode": "path",
    "format": "请查看这张剪贴版图片分析内容\n{path}",
    "path": {
      "mode": "custom_path",
      "dir": "%LOCALAPPDATA%\\ClipVision\\images\\",
      "style": "windows"
    },
    "description": {}
  }
}
```

字段说明：

- `hotkeys`：快捷键绑定列表。
- `hotkeys[].action`：快捷键触发的动作，目前可用 `clipboard_image_path`。
- `hotkeys[].key`：触发快捷键，使用 `+` 分隔按键。
- `behavior.auto_paste`：生成路径后是否自动粘贴到当前窗口。
- `behavior.auto_submit`：是否自动提交，默认建议保持 `false`。
- `output.mode`：输出模式，目前主要使用 `path`。
- `output.format`：粘贴文本格式，`{path}` 会被替换为图片路径。
- `output.path.mode`：图片保存路径模式，支持 `custom_path` 和 `workspace`。
- `output.path.dir`：`custom_path` 模式下的图片保存目录。
- `output.path.style`：路径风格，支持 `windows` 和 `linux`。

项目仍在开发中，功能和配置格式可能会继续变化。
