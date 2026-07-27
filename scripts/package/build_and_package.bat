@echo off
chcp 65001 >nul

REM 切换到项目根目录（脚本位于 scripts/package/）
cd /d "%~dp0\..\.."

echo ========================================
echo    ClipBridge 构建打包脚本
echo ========================================
echo.

REM 设置编译参数
set BUILD_DIR=build
set BUILD_TYPE=Release

REM 清理旧构建
if exist "%BUILD_DIR%" (
    echo [1/4] 清理旧的构建目录...
    rmdir /s /q "%BUILD_DIR%"
)

REM 配置项目
echo [2/4] 配置项目...
cmake -B "%BUILD_DIR%" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if %errorlevel% neq 0 (
    echo [错误] 配置失败！
    pause
    exit /b %errorlevel%
)

REM 编译
echo.
echo [3/4] 编译项目...
cmake --build "%BUILD_DIR%" -j8
if %errorlevel% neq 0 (
    echo [错误] 编译失败！
    pause
    exit /b %errorlevel%
)

REM 打包
echo.
echo [4/4] 打包...
call "%~dp0package.bat"

echo.
echo ========================================
echo    构建打包完成！
echo ========================================
echo.
pause
