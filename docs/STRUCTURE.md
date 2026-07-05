# 项目结构

```
ClipBridge/
├── CMakeLists.txt           # CMake 构建配置
├── ClipBridge.pro           # Qt qmake 项目文件
├── config.json              # 示例配置文件
├── README.md                # 项目主页
├── resources/
│   └── icon.svg             # 应用图标
├── docs/
│   ├── CONTRIBUTING.md      # 贡献指南
│   └── STRUCTURE.md         # 项目结构说明
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
    │   ├── SettingsDialog.cpp
    │   ├── HotkeyEditDialog.h
    │   └── HotkeyEditDialog.cpp
    └── utils/
        ├── Hotkey.h         # 全局热键封装
        ├── Hotkey.cpp
        ├── ClipboardHelper.h # 剪贴板工具
        └── ClipboardHelper.cpp
```

## 模块说明

### core

核心业务逻辑，包括配置管理和动作执行。

- `AppConfig`：配置数据结构、JSON 读写
- `ActionManager`：根据动作名称分发执行具体动作

### ui

用户界面相关，基于 Qt 的图形化界面。

- `TrayIcon`：系统托盘图标和菜单
- `SettingsDialog`：设置主界面
- `HotkeyEditDialog`：热键编辑对话框

### utils

通用工具类。

- `Hotkey`：跨平台全局热键注册
- `ClipboardHelper`：剪贴板读写和模拟粘贴

## 分支说明

| 分支 | 说明 |
|------|------|
| `qt` | Qt 跨平台版本 |
| `master` | Windows 原生版本 |
