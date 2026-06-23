# ClipBridge Qt 版本

这是 ClipBridge 的 Qt 重构版本，代码更简洁，维护更容易。

## 对比

| 指标 | 原版 Win32 | Qt 版本 |
|------|-----------|---------|
| 源文件数 | 10+ | 6 |
| 代码行数 | ~1000 | ~500 |
| 外部依赖 | nlohmann_json, GDI+ | Qt6 内置 |

## 目录结构

```
src-qt/
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

## 构建

### 前置要求

- Qt 6.2+
- CMake 3.16+
- Visual Studio 2019+ (Windows)

### 构建步骤

```bash
# 使用 Qt 版本的 CMakeLists.txt
cp CMakeLists-Qt.txt CMakeLists.txt

# 配置和构建
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6
cmake --build . --config Release
```

### 快速试用

如果你想测试 Qt 版本但不想覆盖现有 CMakeLists.txt：

```bash
mkdir build-qt && cd build-qt
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/msvc2019_64 .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## 保留的设计

- ✅ 配置文件格式 100% 兼容
- ✅ ActionManager 的 Handler 模式
- ✅ 多动作绑定支持

## 新增功能

- 🆕 系统托盘图标
- 🆕 更好的错误日志输出
