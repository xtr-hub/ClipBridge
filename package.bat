@echo off
chcp 65001 >nul
echo ========================================
echo    ClipBridge 打包脚本
echo ========================================
echo.

REM 检查是否存在 build 目录
if not exist "build" (
    echo [错误] 未找到 build 目录，请先编译项目！
    echo 运行以下命令编译：
    echo   mkdir build
    echo   cd build
    echo   cmake .. -G "Visual Studio 17 2022" -A x64
    echo   cmake --build . --config Release
    pause
    exit /b 1
)

REM 检查 Release 编译结果
if not exist "build\Release\ClipBridge.exe" (
    echo [错误] 未找到 ClipBridge.exe，请先编译 Release 版本！
    pause
    exit /b 1
)

REM 创建打包目录
set "PACKAGE_DIR=ClipBridge_Win32"
if exist "%PACKAGE_DIR%" (
    echo [信息] 清理旧的打包目录...
    rmdir /s /q "%PACKAGE_DIR%"
)
mkdir "%PACKAGE_DIR%"

echo [信息] 复制文件...

REM 复制主程序
copy "build\Release\ClipBridge.exe" "%PACKAGE_DIR%\" >nul

REM 复制配置文件
copy "config.json" "%PACKAGE_DIR%\" >nul

REM 复制文档
copy "README.md" "%PACKAGE_DIR%\" >nul
copy "LICENSE" "%PACKAGE_DIR%\" >nul

REM 获取版本号（从 git tag 或使用日期）
for /f "tokens=*" %%i in ('git describe --tags --abbrev=0 2^>nul') do set "VERSION=%%i"
if "%VERSION%"=="" set "VERSION=%date:~0,4%%date:~5,2%%date:~8,2%"

REM 创建 zip 压缩包
set "ZIP_NAME=ClipBridge_Win32_%VERSION%.zip"
if exist "%ZIP_NAME%" del "%ZIP_NAME%"

echo [信息] 创建压缩包: %ZIP_NAME%...
powershell -Command "Compress-Archive -Path '%PACKAGE_DIR%\*' -DestinationPath '%ZIP_NAME%' -Force"

echo.
echo ========================================
echo    打包完成！
echo ========================================
echo 输出目录: %PACKAGE_DIR%
echo 压缩包: %ZIP_NAME%
echo.

REM 打开打包目录
explorer "%PACKAGE_DIR%"

pause
