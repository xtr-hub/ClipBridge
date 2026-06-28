@echo off
chcp 65001 >nul
echo ========================================
echo    ClipBridge Qt 版本打包脚本
echo ========================================
echo.

REM 检查是否存在 build 目录
if not exist "build" (
    echo [错误] 未找到 build 目录，请先编译项目！
    echo 运行以下命令编译：
    echo   cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
    echo   cmake --build build
    pause
    exit /b 1
)

REM 检测编译输出路径（支持 MinGW 和 MSVC）
set "EXE_PATH="
if exist "build\ClipBridge.exe" (
    set "EXE_PATH=build\ClipBridge.exe"
    echo [信息] 检测到 MinGW 编译输出
) else if exist "build\Release\ClipBridge.exe" (
    set "EXE_PATH=build\Release\ClipBridge.exe"
    echo [信息] 检测到 MSVC 编译输出
) else (
    echo [错误] 未找到 ClipBridge.exe，请先编译！
    echo 查找路径：
    echo   - build\ClipBridge.exe (MinGW)
    echo   - build\Release\ClipBridge.exe (MSVC)
    pause
    exit /b 1
)

REM 查找 Qt 目录
set "QT_DIR="
if exist "D:\tools\qt\6.11.1\mingw_64\bin\windeployqt.exe" (
    set "QT_DIR=D:\tools\qt\6.11.1\mingw_64"
) else if exist "C:\Qt\6.5.0\msvc2019_64\bin\windeployqt.exe" (
    set "QT_DIR=C:\Qt\6.5.0\msvc2019_64"
) else if exist "C:\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe" (
    set "QT_DIR=C:\Qt\5.15.2\msvc2019_64"
) else (
    echo [警告] 未自动找到 Qt 目录，请手动设置 QT_DIR 环境变量
    echo 例如: set QT_DIR=C:\Qt\6.5.0\msvc2019_64
    echo.
)

if not "%QT_DIR%"=="" (
    echo [信息] 使用 Qt 目录: %QT_DIR%
    set "PATH=%QT_DIR%\bin;%PATH%"
)

REM 创建打包目录
set "PACKAGE_DIR=ClipBridge_Qt"
if exist "%PACKAGE_DIR%" (
    echo [信息] 清理旧的打包目录...
    rmdir /s /q "%PACKAGE_DIR%"
)
mkdir "%PACKAGE_DIR%"

echo [信息] 复制文件...

REM 复制主程序
copy "%EXE_PATH%" "%PACKAGE_DIR%\" >nul

REM 复制配置文件
copy "config.json" "%PACKAGE_DIR%\" >nul

REM 复制文档
copy "README.md" "%PACKAGE_DIR%\" >nul
copy "LICENSE" "%PACKAGE_DIR%\" >nul

REM 使用 windeployqt 自动打包 Qt 依赖
if exist "%PACKAGE_DIR%\ClipBridge.exe" (
    echo [信息] 使用 windeployqt 打包 Qt 依赖...
    where windeployqt >nul 2>&1
    if errorlevel 1 (
        echo [警告] 未找到 windeployqt，尝试从已知路径...
        if exist "%QT_DIR%\bin\windeployqt.exe" (
            "%QT_DIR%\bin\windeployqt.exe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw "%PACKAGE_DIR%\ClipBridge.exe"
        ) else (
            echo [错误] 无法找到 windeployqt！
            echo 请确保已安装 Qt 并正确设置 QT_DIR
            pause
            exit /b 1
        )
    ) else (
        windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw "%PACKAGE_DIR%\ClipBridge.exe"
    )
)

REM 获取版本号 - 优先从 CMakeLists.txt 读取
set "VERSION="
if exist "CMakeLists.txt" (
    for /f "tokens=3" %%i in ('findstr /r "project.*VERSION" CMakeLists.txt') do (
        set "VERSION=%%i"
    )
)
if defined VERSION (
    echo [信息] 从 CMakeLists.txt 读取版本: %VERSION%
) else (
    REM 尝试从 git tag 获取
    for /f "tokens=*" %%i in ('git describe --tags --abbrev=0 2^>nul') do set "VERSION=%%i"
    if defined VERSION (
        echo [信息] 从 git tag 读取版本: %VERSION%
    ) else (
        REM 使用日期作为后备
        set "VERSION=%date:~0,4%%date:~5,2%%date:~8,2%"
        echo [信息] 使用日期作为版本: %VERSION%
    )
)

REM 创建 zip 压缩包
set "ZIP_NAME=ClipBridge_Qt_%VERSION%.zip"
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

explorer "%PACKAGE_DIR%"

pause
