# ClipBridge

Using Claude in a terminal is great—until you need to send an image or a file. You end up saving it, finding the path, copying, pasting back into the terminal. ClipBridge handles this in one hotkey: screenshot, press, paste the path into Claude.

English | [中文](README.md)

## Sending Screenshots to Claude

You've taken a screenshot and want Claude to look at it. No file manager, no manual path copying.

**Option one: direct hotkey**

Press `Ctrl+Alt+I` after taking a screenshot. The image auto-saves as PNG and the file path lands on your clipboard. Switch to Claude, `Ctrl+V`, done.

**Option two: preview and add a prompt**

Press `Ctrl+Alt+Space` to open the Input Panel. If there's an image on the clipboard, the preview bar at the top shows a thumbnail card—thumbnail on the left, dimensions in the middle (e.g. 1920x1080), and an × button on the right to remove it. Type a prompt in the text box below, like "analyze the error in this screenshot", then `Ctrl+Enter`. The prompt and image path are concatenated on your clipboard, ready to paste into Claude.

You can customize the output format in settings. For example, set it to `Check this image: {path}` and `{path}` gets replaced with the actual file path every time.

## Sending Files to Claude

Copy files in your file manager, press `Ctrl+Alt+F`, and all file paths land on your clipboard separated by newlines. Paste into Claude—one path per line.

If you want to review the file list, remove unwanted items, or mix files with screenshots, use the Input Panel:

1. Select files in your file manager, `Ctrl+C`
2. Press `Ctrl+Alt+Space` to open the Input Panel
3. `Ctrl+V` inside the panel—files appear as compact cards in the preview bar, each showing the system file icon and filename (long names get elided)
4. Take screenshots, `Ctrl+V` more content—images and files mix freely
5. Click × on any card you don't want
6. Type your prompt, `Ctrl+Enter` to send

The preview bar scrolls horizontally. Cards are 106 pixels wide, about 5 visible at once. Mouse wheel scrolls left/right. Each card tracks its position by property, so removing the first card re-indexes the rest correctly.

## Stripping Newlines

Multi-line text copied from a terminal breaks formatting when pasted elsewhere:

```
sudo apt update &&
sudo apt upgrade -y &&
sudo apt autoremove
```

Press `Ctrl+Alt+J` and it becomes:

```
sudo apt update && sudo apt upgrade -y && sudo apt autoremove
```

Paste it anywhere—no manual editing needed.

You can also do this in the Input Panel. Paste text into the editor, check "Pin" to keep the window open, `Ctrl+Enter` to send, and the panel stays ready for the next batch.

## Sending Long Text

Sometimes you need to send Claude a lot of text—hundreds of lines of logs, a full config file, a large code block. Pasting it directly might overflow the input.

Copy the text and press `Ctrl+Alt+L`. If it exceeds the threshold (500 characters by default), it auto-saves as a .txt file and the file path goes to your clipboard. Claude reads the file from the path.

Adjust the threshold in settings. If your model supports longer inputs, raise it to 2000 or more. Set it to 0 to disable file-saving entirely—text always outputs directly.

This also works in the Input Panel. Check "Save long text as file", paste your text, type a prompt, and `Ctrl+Enter`. The output is a file path instead of raw text.

## The Input Panel

The Input Panel is your staging area between the clipboard and Claude. Press `Ctrl+Alt+Space` to open a floating dark-themed window.

**Preview Area**

A 26-pixel bar at the top. Content you paste appears as horizontal cards. Image cards show a thumbnail; file cards use the platform's native file icon. Each card has an × to dismiss.

Mouse wheel on the preview area scrolls horizontally—wheel events are explicitly redirected since there's no vertical scrollbar. Powered by Qt's native scrollbar, no custom implementation.

**Text Editor**

The large central text area. Supports IME input. `Ctrl+Enter` to send, `Ctrl+V` to paste into the preview area (images/files) or the editor itself (plain text). Placeholder text shows available shortcuts.

Typing doesn't clear the preview. Images, files, and your typed prompt coexist—on send, images and files come first, then your prompt.

**Bottom Controls**

Two checkboxes plus Cancel and Send buttons.

Pin mode: the window stays open after sending, all inputs are cleared, and focus returns to the editor. Perfect for batch operations—screenshot, send, screenshot, send, without reopening the panel.

Save long text as file: text in the editor exceeding the threshold gets auto-saved as a .txt file.

## Monitor Mode

Right-click tray → Monitor Mode. Once enabled, you don't press anything.

Every time something hits the clipboard—a screenshot, copied files, copied text—ClipBridge detects the content type, matches it against the auto-trigger actions you selected in settings, runs them, and writes the result back to the clipboard.

For example, if you've checked "Copy Image Path" and "Strip Newlines": screenshots auto-generate paths, and multi-line text auto-cleans itself.

Monitor mode automatically pauses when the Input Panel is open (to avoid processing clipboard changes you're making inside the panel) and resumes when it closes. The pause mechanism uses reference counting, so future components that also need to pause monitoring won't interfere with each other.

## Settings

Right-click the tray icon → Settings. VSCode dark theme, left navigation, right content. All changes take effect immediately when you close the window.

**Shortcuts**

Lists all current hotkeys: action name, key combination, auto-paste, auto-submit. Edit and delete buttons on each row. Add button at the bottom.

When adding a hotkey, the action dropdown only shows unassigned actions—one action, one hotkey. If all four actions are already bound, pressing Add shows a warning.

The key input uses a QKeySequenceEdit—click it and press your desired combination, no manual typing. Conflicts with existing hotkeys are caught and reported.

The Input Panel shortcut gets its own setting at the bottom, with a reset button to clear it.

**Output Format**

Global default format and per-action overrides. `{path}` is the placeholder for image/file/text save paths, `{text}` for raw clipboard text.

Save mode "workspace" uses the system temp directory (`%TEMP%/ClipBridge Images` or `ClipBridge Texts` on Windows), with auto-created folders. "Custom path" lets you pick any directory.

Paste key supports `Ctrl+V` and `Shift+Insert`. Paste delay controls how many milliseconds to wait before restoring the original clipboard content after an auto-paste.

**Monitor Mode**

Toggle and auto-trigger action checkboxes. Long text threshold via a slider, 0 to 10000 range. At 0 the slider label shows "Disabled".

**General**

Language switch (Chinese/English), takes effect on next settings window open. Auto-start checkbox: writes to Registry on Windows, LaunchAgent on macOS, autostart file on Linux. Window opacity slider, range 50% to 100%—the lower bound prevents making the window invisible.

## Install and Build

Download pre-built binaries from [Releases](https://github.com/xtr-hub/ClipBridge/releases), extract, and run.

To build from source, you need Qt 5.15+ or Qt 6, and CMake 3.16+:

```bash
# Windows (MinGW)
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8

# macOS
brew install qt@5
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/usr/local/opt/qt@5
cmake --build . --config Release

# Linux
sudo apt install qtbase5-dev libx11-dev libxtst-dev
mkdir build && cd build
cmake .. && cmake --build . --config Release
```

Running tests:

```bash
cmake -B build -DBUILD_TESTS=ON
cmake --build build -j8
ctest --test-dir build --output-on-failure
```

## Default Hotkeys

| Hotkey | Action | Result |
|--------|--------|--------|
| `Ctrl+Alt+I` | Copy Image Path | Clipboard image → saved PNG → path on clipboard |
| `Ctrl+Alt+F` | Copy File Path | Clipboard files → path list on clipboard |
| `Ctrl+Alt+J` | Strip Newlines | Multi-line text → single line |
| `Ctrl+Alt+Space` | Input Panel | Open preview/edit window |
| unset | Long Text to File | Bind in settings or trigger via Input Panel |

All hotkeys are customizable. One action per hotkey, no conflicts.

## FAQ

**Do the Input Panel and Monitor Mode conflict?**
No. Monitor pauses when the panel opens, resumes when it closes.

**Can one action have multiple hotkeys?**
No. One-to-one mapping ensures one key does one thing.

**What threshold should I use for long text?**
Default is 500 characters. Raise it for models with larger context windows. Set to 0 to disable file-saving entirely.

**Where are images saved?**
Default is the `ClipBridge Images` folder in your system temp directory. Change it in Settings → Output Format.

**What if a hotkey conflicts with other software?**
Change it in Settings. Click the key input and press your new combination.

**Global hotkeys don't work on Linux?**
X11 is required. Wayland support depends on your compositor—X11 is recommended.

**What permissions does macOS need?**
Accessibility permission, prompted on first launch. Grant it in System Settings → Privacy & Security → Accessibility.
