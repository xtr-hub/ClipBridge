# ClipBridge Qt 版本打包脚本 (PowerShell)
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   ClipBridge Qt 版本打包脚本" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 检查是否存在 build 目录
if (-not (Test-Path "build")) {
    Write-Host "[错误] 未找到 build 目录，请先编译项目！" -ForegroundColor Red
    Write-Host "运行以下命令编译：" -ForegroundColor Yellow
    Write-Host "  mkdir build"
    Write-Host "  cd build"
    Write-Host "  cmake .. -G 'Visual Studio 17 2022' -A x64"
    Write-Host "  cmake --build . --config Release"
    Read-Host "按回车键退出"
    exit 1
}

# 检查 Release 编译结果
if (-not (Test-Path "build\Release\ClipBridge.exe")) {
    Write-Host "[错误] 未找到 ClipBridge.exe，请先编译 Release 版本！" -ForegroundColor Red
    Read-Host "按回车键退出"
    exit 1
}

# 查找 Qt 目录
$qtDir = ""
$qtPaths = @(
    "D:\tools\qt\6.11.1\mingw_64",
    "C:\Qt\6.5.0\msvc2019_64",
    "C:\Qt\5.15.2\msvc2019_64"
)

foreach ($path in $qtPaths) {
    if (Test-Path "$path\bin\windeployqt.exe") {
        $qtDir = $path
        break
    }
}

if (-not $qtDir -and $env:QT_DIR) {
    $qtDir = $env:QT_DIR
}

if (-not $qtDir) {
    Write-Host "[警告] 未自动找到 Qt 目录，请手动设置 QT_DIR 环境变量" -ForegroundColor Yellow
    Write-Host "例如: `$env:QT_DIR = 'C:\Qt\6.5.0\msvc2019_64'" -ForegroundColor Gray
    Write-Host ""
} else {
    Write-Host "[信息] 使用 Qt 目录: $qtDir" -ForegroundColor Cyan
    $env:PATH = "$qtDir\bin;$env:PATH"
}

# 创建打包目录
$packageDir = "ClipBridge_Qt"
if (Test-Path $packageDir) {
    Write-Host "[信息] 清理旧的打包目录..." -ForegroundColor Yellow
    Remove-Item -Path $packageDir -Recurse -Force
}
New-Item -Path $packageDir -ItemType Directory | Out-Null

Write-Host "[信息] 复制文件..." -ForegroundColor Cyan

# 复制主程序和已有的 DLL
Copy-Item -Path "build\Release\*" -Destination "$packageDir\" -Recurse -Force

# 复制配置文件
Copy-Item "config.json" -Destination "$packageDir\" -Force

# 复制文档
Copy-Item "README.md" -Destination "$packageDir\" -Force
Copy-Item "LICENSE" -Destination "$packageDir\" -Force

# 使用 windeployqt 自动打包 Qt 依赖
$exePath = Join-Path $packageDir "ClipBridge.exe"
if (Test-Path $exePath) {
    Write-Host "[信息] 使用 windeployqt 打包 Qt 依赖..." -ForegroundColor Cyan

    $windeployqt = Get-Command "windeployqt.exe" -ErrorAction SilentlyContinue
    if (-not $windeployqt -and $qtDir) {
        $windeployqtPath = Join-Path $qtDir "bin\windeployqt.exe"
        if (Test-Path $windeployqtPath) {
            $windeployqt = $windeployqtPath
        }
    }

    if ($windeployqt) {
        & $windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw $exePath
    } else {
        Write-Host "[错误] 无法找到 windeployqt！" -ForegroundColor Red
        Write-Host "请确保已安装 Qt 并正确设置 QT_DIR" -ForegroundColor Red
        Read-Host "按回车键退出"
        exit 1
    }
}

# 获取版本号
try {
    $version = git describe --tags --abbrev=0 2>$null
} catch {
    $version = ""
}
if (-not $version) {
    $version = Get-Date -Format "yyyyMMdd"
}

# 创建 zip 压缩包
$zipName = "ClipBridge_Qt_$version.zip"
if (Test-Path $zipName) {
    Remove-Item $zipName -Force
}

Write-Host "[信息] 创建压缩包: $zipName..." -ForegroundColor Cyan
Compress-Archive -Path "$packageDir\*" -DestinationPath $zipName -Force

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "   打包完成！" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "输出目录: $packageDir" -ForegroundColor White
Write-Host "压缩包: $zipName" -ForegroundColor White
Write-Host ""

# 打开打包目录
explorer $packageDir
