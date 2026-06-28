# ClipBridge Win32 构建打包脚本 (PowerShell)
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   ClipBridge Win32 构建打包脚本" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 设置编译参数
$BUILD_DIR = "build"
$BUILD_TYPE = "Release"

# 清理旧构建
if (Test-Path $BUILD_DIR) {
    Write-Host "[1/4] 清理旧的构建目录..." -ForegroundColor Yellow
    Remove-Item -Path $BUILD_DIR -Recurse -Force
}

# 配置项目
Write-Host "[2/4] 配置项目..." -ForegroundColor Yellow
cmake -B $BUILD_DIR -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) {
    Write-Host "[错误] 配置失败！" -ForegroundColor Red
    Read-Host "按回车键退出"
    exit $LASTEXITCODE
}

# 编译
Write-Host ""
Write-Host "[3/4] 编译项目..." -ForegroundColor Yellow
cmake --build $BUILD_DIR --config $BUILD_TYPE
if ($LASTEXITCODE -ne 0) {
    Write-Host "[错误] 编译失败！" -ForegroundColor Red
    Read-Host "按回车键退出"
    exit $LASTEXITCODE
}

# 打包
Write-Host ""
Write-Host "[4/4] 打包..." -ForegroundColor Yellow
powershell -ExecutionPolicy Bypass -File package.ps1

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "   构建打包完成！" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Read-Host "按回车键退出"
