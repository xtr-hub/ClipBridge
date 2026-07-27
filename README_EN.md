<div align="center">
  <img src="resources/icon.png" alt="logo" width="200" height="200">

  # ClipBridge

  Cross-platform hotkey utility that makes clipboard interactions with Claude TUI / Claude Code smoother

  <div style="display: flex; justify-content: center; gap: 12px; margin-bottom: 12px; flex-wrap: wrap;">
    <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-blue.svg?style=flat&logo=github" alt="License"></a>
    <a href="#platform-support"><img src="https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg?style=flat" alt="Platform"></a>
    <a href="https://github.com/xtr-hub/ClipBridge/releases"><img src="https://img.shields.io/badge/Download-Releases-green.svg?style=flat&logo=github" alt="Releases"></a>
  </div>
</div>

English | [中文](README.md)

## What It Does

- **Image-to-TUI magic**: take a screenshot → press a hotkey → image is auto-saved → its path is auto-pasted → send instantly
- **One-click file path copy**: copy files → press a hotkey → file paths are auto-pasted
- **One-click newline removal**: copy a multi-line command from the terminal, newlines are stripped automatically, ready to use
- **Custom output format**: supports `{path}` placeholders, each action can have its own pasted text
- **GUI hotkey manager**: add/edit/delete hotkeys, each with independent auto-paste and auto-submit settings
- **Auto hot-reload**: changes take effect immediately after saving config, no restart needed
- **Cross-platform**: Windows / macOS / Linux

## Quick Start

### Download and Run

1. Download a prebuilt binary from [Releases](https://github.com/xtr-hub/ClipBridge/releases)
2. Or build from source (see below)
3. Run the program; the ClipBridge icon appears in the system tray

### Basic Usage

1. Take a screenshot → press `Ctrl+Alt+I` → the image path is pasted into the current input field
2. Copy files → press `Ctrl+Alt+F` → the file paths are pasted into the current input field
3. Copy a multi-line command → press `Ctrl+Alt+J` → newlines are stripped and it's ready to execute

### GUI Settings

**Left/right click the tray icon** → open the settings window

**Settings window features:**

1. **Hotkey management**
   - View the existing hotkey list (shows action, key, and behavior config)
   - Add new hotkeys
   - Edit existing hotkeys (action, key, behavior config)
   - Delete hotkeys

2. **Output settings**
   - Configure the global default output format (supports `{path}` placeholder)
   - Configure a per-action output format; unconfigured actions fall back to the global format
   - Choose save mode: desktop default path / custom path
   - Graphically select a custom save directory

3. **Default behavior config**
   - Configure default behavior for newly added hotkeys
   - Auto-paste
   - Auto-submit

4. **Auto hot-reload after saving**
   - No program restart required; hotkey config takes effect immediately

---

## Configuration

The configuration file `config.json` is located in the same directory as the program:

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
      "clipboard_image_path": "Check this clipboard image {path}",
      "clipboard_file_path": "Check this file {path}"
    },
    "mode": "workspace",
    "dir": ""
  }
}
```

### Field Reference

| Field | Type | Description |
|-------|------|-------------|
| `hotkeys` | array | List of hotkey bindings |
| `hotkeys[].action` | string | Action: `clipboard_image_path`, `clipboard_file_path`, or `strip_newlines` |
| `hotkeys[].key` | string | Hotkey, separated by `+` (e.g. `ctrl+alt+i`) |
| `hotkeys[].behavior` | object | Optional per-hotkey behavior |
| `default_behavior` | object | Default behavior config |
| `default_behavior.auto_paste` | bool | Whether to auto-paste after the action |
| `default_behavior.auto_submit` | bool | Whether to auto-submit (use with caution) |
| `output.format` | string | Global default output format; `{path}` is replaced with the path |
| `output.formats` | object | Per-action output formats; falls back to `output.format` when not set |
| `output.mode` | string | `workspace` (desktop) or `custom_path` (custom directory) |
| `output.dir` | string | Save directory when `custom_path` mode is used |

---

## Branches

> **Important**: both versions have the same features; they differ only in tech stack

| Branch | Tech Stack | Description | Recommended When |
|--------|------------|-------------|------------------|
| **qt** | Qt 6/5 | Cross-platform version (current) | You need macOS/Linux support |
| **win32** | Win32 API | Native Windows version | You only need Windows and want something lighter |

---

## Build Guide

### Prerequisites

- **Qt 5.15.2+** or **Qt 6**
- **CMake 3.16+**
- Platform compiler:
  - Windows: Visual Studio 2019+ or MinGW
  - macOS: Clang (Xcode Command Line Tools)
  - Linux: GCC/Clang

---

### Windows Build

1. **Download Qt**

   Visit <https://www.qt.io/download> to install Qt

   - **Recommended**: Qt 6.11.1 (LTS)
   - Choose a version:
     - `msvc2019_64` / `msvc2022_64` (for Visual Studio)
     - `mingw_64` (for MinGW compiler)

2. **Configure Qt path**

   Edit `CMakeLists.txt` and add your Qt installation path (common paths are already configured by default):

   ```cmake
   list(APPEND CMAKE_PREFIX_PATH
       # Change to your actual path
       "C:/Qt/6.11.1/msvc2019_64"
       "D:/tools/qt/6.11.1/mingw_64"
   )
   ```

3. **Build the project**

   Option 1: MinGW (recommended)
   ```bash
   cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j8
   ```

   Option 2: Visual Studio
   ```bash
   mkdir build && cd build
   cmake -G "Visual Studio 17 2022" -A x64 ..
   cmake --build . --config Release
   ```

   After building, the executable is in `build/` (MinGW) or `build/Release/` (Visual Studio).
   CMake will automatically run `windeployqt` to copy Qt DLLs to the output directory.

4. **One-click build and package (recommended)**

   The project includes one-click build/package scripts:
   ```cmd
   # Using batch script
   scripts/package/build_and_package.bat

   # Or using PowerShell script
   powershell -ExecutionPolicy Bypass -File scripts/package/build_and_package.ps1
   ```

5. **Package for release**

   If you have already built manually and only want to package:
   ```cmd
   # Using batch script
   scripts/package/package.bat

   # Or using PowerShell script
   powershell -ExecutionPolicy Bypass -File scripts/package/package.ps1
   ```

   The script creates a `ClipBridge_Qt` folder with all dependencies and a zip archive.

---

### macOS Build

```bash
# Install Qt
brew install qt@5

# Build
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/usr/local/opt/qt@5
cmake --build . --config Release
```

**Package**

```bash
./scripts/package/package_macos.sh
```

The script creates `ClipBridge.app` and `ClipBridge_macOS_<version>.zip`.

**Note**: on first run, add the program to "System Preferences → Security & Privacy → Accessibility".

---

### Linux Build

```bash
# Install dependencies
sudo apt install qtbase5-dev libx11-dev libxtst-dev

# Build
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

---

## Platform Support

| Platform | Hotkeys | Paste | Notes |
|----------|---------|-------|-------|
| Windows | ✅ | ✅ | Full support |
| Linux X11 | ✅ | ✅ | Requires X11 |
| macOS | ✅ | ✅ | Requires Accessibility permission |

---

## Project Structure

See [Project Structure](docs/STRUCTURE.md).

---

## FAQ

### Q: How do I find my Qt path?

A: Qt is installed by default at:
- Windows: `C:/Qt/` or `D:/Qt/`
- macOS: `/usr/local/opt/qt@5`
- Linux: `/usr/include/qt5`

You can also check "Tools → Options → Kits" in Qt Creator.

### Q: Compilation error "Qt requires a C++17 compiler"

A: Add the following to CMakeLists.txt:

```cmake
if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()
```

### Q: Compilation warning "The source file contains characters that cannot be represented in the current code page"

A: Add the following to CMakeLists.txt:

```cmake
if(MSVC)
    add_compile_options(/utf-8)
endif()
```

### Q: Hotkeys don't work on macOS

A: Open "System Preferences → Security & Privacy → Accessibility", add and check ClipBridge.

### Q: How do I customize hotkeys?

A: Two ways:
1. **Recommended: use the GUI settings window (click tray icon → open settings)**
2. Manually edit the `config.json` file

---

## Contributing

Issues and Pull Requests are welcome!

Please read the [Contributing Guide](docs/CONTRIBUTING_EN.md) before contributing.

Thanks to everyone who has contributed to ClipBridge!

---

## License

This project is licensed under the MIT License. See the LICENSE file for details.
