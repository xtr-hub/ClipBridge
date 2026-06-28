# Release 1.0.0

## 🎉 首次正式发布

ClipBridge 1.0.0 终于来了！让 Claude TUI / Claude Code 的剪贴板交互更顺畅。

---

## ✨ 核心功能

### 📸 clipboard_image_path —— TUI 发图神器
在 Claude TUI / Claude Code 里发截图，不再需要手动保存文件、复制路径！
- 截图 → 按快捷键 → 自动保存图片 → 自动粘贴路径 → 直接发送
- 支持自定义格式文案（比如 `请查看这张图片：{path}`）
- 支持自定义保存路径

### 📝 strip_newlines —— 清理换行符
从终端、日志、文档复制多行文本时，经常因为终端宽度限制被自动插入换行符，导致粘贴后无法直接执行。
- 按快捷键，一键去除所有换行符
- 命令即贴即用，无需手动清理

### ⌨️ 每个热键独立配置
- 每个快捷键可以设置独立的 `auto_paste` 和 `auto_submit` 行为
- 支持多动作绑定，同一个快捷键按顺序执行多个动作

---

## 🖥️ 平台支持

| 平台 | 热键 | 粘贴 | 说明 |
|------|------|------|------|
| Windows | ✅ | ✅ | 完整支持 |
| macOS | ✅ | ✅ | 需要辅助功能权限 |
| Linux | 🔄 | 🔄 | 开发中 |

---

## 🚀 快速开始

### Windows
1. 下载 `ClipBridge_Windows_1.0.0.zip`
2. 解压到任意目录
3. 运行 `ClipBridge.exe`
4. 系统托盘出现 ClipBridge 图标

### macOS
1. 下载 `ClipBridge_macOS_1.0.0.zip`
2. 解压得到 `ClipBridge.app`
3. 拖动到应用程序文件夹
4. 首次运行需要在"系统偏好设置 → 安全性与隐私 → 辅助功能"中添加并勾选 ClipBridge
5. 运行程序

### 基本使用
- 截图 → 按 `Ctrl+Alt+I` (Windows) / `Cmd+Option+I` (macOS) → 图片路径自动粘贴
- 复制多行命令 → 按 `Ctrl+Alt+J` (Windows) / `Cmd+Option+J` (macOS) → 换行符已清理

### 图形界面设置
- 左键/右键点击托盘图标 → 打开设置界面
- 可以查看和管理热键、配置图片保存格式和路径

---

## 📦 下载

- **Windows**: `ClipBridge_Windows_1.0.0.zip`
- **macOS**: `ClipBridge_macOS_1.0.0.zip`

---

## 🎨 本次更新

### 1.0.0
- ✅ 支持 Windows 和 macOS 双平台
- ✅ 实现 clipboard_image_path 功能
- ✅ 实现 strip_newlines 功能
- ✅ 系统托盘图标
- ✅ 图形化设置界面
- ✅ 每个热键独立配置
- ✅ 支持自定义图片保存格式
- ✅ GitHub Actions CI 自动构建
- ✅ 自定义 SVG 图标

---

## ⚠️ 注意事项

### macOS
首次运行需要辅助功能权限，请在"系统偏好设置 → 安全性与隐私 → 辅助功能"中添加并勾选 ClipBridge。

### Windows
首次运行可能会被 SmartScreen 拦截，请点击"更多信息" → "仍要运行"。

---

## 📄 许可证

MIT License

---

Made with ❤️ for Claude TUI users
