# ClipBridge 打包脚本 (PowerShell)
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   ClipBridge 打包脚本" -ForegroundColor Cyan
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

# 创建打包目录
$packageDir = "ClipBridge_Win32"
if (Test-Path $packageDir) {
    Write-Host "[信息] 清理旧的打包目录..." -ForegroundColor Yellow
    Remove-Item -Path $packageDir -Recurse -Force
}
New-Item -Path $packageDir -ItemType Directory | Out-Null

Write-Host "[信息] 复制文件..." -ForegroundColor Cyan

# 复制主程序
Copy-Item "build\Release\ClipBridge.exe" -Destination "$packageDir\"

# 复制配置文件
Copy-Item "config.json" -Destination "$packageDir\"

# 复制文档
Copy-Item "README.md" -Destination "$packageDir\"
Copy-Item "LICENSE" -Destination "$packageDir\"

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
$zipName = "ClipBridge_Win32_$version.zip"
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
