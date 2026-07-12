#!/bin/bash

# ClipBridge macOS Packaging Script

set -e

# 切换到项目根目录（脚本位于 scripts/package/）
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/../.."

PROJECT_ROOT="$(pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
APP_NAME="ClipBridge"
APP_DIR="${APP_NAME}.app"

echo "========================================"
echo "   ClipBridge macOS 打包脚本"
echo "========================================"
echo ""

# 检查构建输出
if [ ! -f "$BUILD_DIR/ClipBridge" ]; then
    echo "[错误] 未找到构建输出：$BUILD_DIR/ClipBridge"
    echo "请先编译项目："
    echo "  cmake -B build -DCMAKE_BUILD_TYPE=Release"
    echo "  cmake --build build -j8"
    exit 1
fi

# 读取版本号
VERSION=""
if [ -f "CMakeLists.txt" ]; then
    VERSION=$(sed -n 's/project(ClipBridge VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt)
fi

if [ -z "$VERSION" ]; then
    VERSION=$(git describe --tags --abbrev=0 2>/dev/null || true)
fi

if [ -z "$VERSION" ]; then
    VERSION=$(date +%Y%m%d)
    echo "[信息] 使用日期作为版本: $VERSION"
else
    echo "[信息] 版本: $VERSION"
fi

# 清理旧包
if [ -d "$APP_DIR" ]; then
    echo "[信息] 清理旧的 App bundle..."
    rm -rf "$APP_DIR"
fi

# 创建 App bundle 结构
echo "[信息] 创建 App bundle..."
mkdir -p "$APP_DIR/Contents/MacOS"
mkdir -p "$APP_DIR/Contents/Resources"

# 复制可执行文件
cp "$BUILD_DIR/ClipBridge" "$APP_DIR/Contents/MacOS/"

# 复制资源文件
cp "config.json" "$APP_DIR/Contents/Resources/"
cp "README.md" "$APP_DIR/Contents/Resources/"
if [ -f "LICENSE" ]; then
    cp "LICENSE" "$APP_DIR/Contents/Resources/"
fi

# 复制图标（如果存在）
if [ -f "resources/icon.icns" ]; then
    cp "resources/icon.icns" "$APP_DIR/Contents/Resources/"
fi

# 创建 Info.plist
cat > "$APP_DIR/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>ClipBridge</string>
    <key>CFBundleIdentifier</key>
    <string>com.xtrhub.clipbridge</string>
    <key>CFBundleName</key>
    <string>ClipBridge</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>${VERSION}</string>
    <key>CFBundleVersion</key>
    <string>${VERSION}</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.14</string>
    <key>NSAccessibilityUsageDescription</key>
    <string>ClipBridge needs accessibility access to listen for global hotkeys.</string>
</dict>
</plist>
EOF

# 使用 macdeployqt 打包 Qt 依赖
echo "[信息] 使用 macdeployqt 打包 Qt 依赖..."
MACDEPLOYQT=""

if [ -n "$Qt5_DIR" ]; then
    if [ -f "$Qt5_DIR/bin/macdeployqt" ]; then
        MACDEPLOYQT="$Qt5_DIR/bin/macdeployqt"
    elif [ -f "$Qt5_DIR/bin/macdeployqt.app/Contents/MacOS/macdeployqt" ]; then
        MACDEPLOYQT="$Qt5_DIR/bin/macdeployqt.app/Contents/MacOS/macdeployqt"
    fi
fi

if [ -z "$MACDEPLOYQT" ]; then
    for path in \
        "/usr/local/opt/qt@5/bin/macdeployqt" \
        "/usr/local/opt/qt5/bin/macdeployqt" \
        "/opt/homebrew/opt/qt@5/bin/macdeployqt" \
        "/opt/homebrew/opt/qt5/bin/macdeployqt" \
        "/usr/local/Qt5/bin/macdeployqt"; do
        if [ -f "$path" ]; then
            MACDEPLOYQT="$path"
            break
        fi
    done
fi

if [ -n "$MACDEPLOYQT" ]; then
    echo "[信息] 找到 macdeployqt: $MACDEPLOYQT"
    "$MACDEPLOYQT" "$APP_DIR"
else
    echo "[警告] 未找到 macdeployqt，将跳过 Qt 依赖自动打包"
    echo "      请确保目标机器已安装 Qt 运行时"
fi

# 创建 zip 压缩包
ZIP_NAME="ClipBridge_macOS_${VERSION}.zip"
if [ -f "$ZIP_NAME" ]; then
    rm -f "$ZIP_NAME"
fi

echo "[信息] 创建压缩包: $ZIP_NAME..."
ditto -c -k --sequesterRsrc --keepParent "$APP_DIR" "$ZIP_NAME"

echo ""
echo "========================================"
echo "   打包完成！"
echo "========================================"
echo "App bundle: $APP_DIR"
echo "压缩包:     $ZIP_NAME"
echo ""
