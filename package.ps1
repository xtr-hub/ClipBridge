# ClipBridge Qt Packaging Script (PowerShell)
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   ClipBridge Qt Packaging Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if build directory exists
if (-not (Test-Path "build")) {
    Write-Host "[ERROR] Build directory not found! Please compile first." -ForegroundColor Red
    Write-Host "Run these commands to compile:" -ForegroundColor Yellow
    Write-Host "  cmake -B build -G `"MinGW Makefiles`" -DCMAKE_BUILD_TYPE=Release"
    Write-Host "  cmake --build build"
    Read-Host "Press Enter to exit"
    exit 1
}

# Detect build output path (support MinGW and MSVC)
$exePath = ""
if (Test-Path "build\ClipBridge.exe") {
    $exePath = "build\ClipBridge.exe"
    Write-Host "[INFO] Detected MinGW build" -ForegroundColor Green
} elseif (Test-Path "build\Release\ClipBridge.exe") {
    $exePath = "build\Release\ClipBridge.exe"
    Write-Host "[INFO] Detected MSVC build" -ForegroundColor Green
} else {
    Write-Host "[ERROR] ClipBridge.exe not found! Please compile first." -ForegroundColor Red
    Write-Host "Looking for:" -ForegroundColor Yellow
    Write-Host "  - build\ClipBridge.exe (MinGW)"
    Write-Host "  - build\Release\ClipBridge.exe (MSVC)"
    Read-Host "Press Enter to exit"
    exit 1
}

# Find Qt directory
$qtDir = ""
$qtPaths = @(
    "D:\tools\qt\6.11.1\mingw_64",
    "C:\Qt\6.5.0\msvc2019_64",
    "C:\Qt\5.15.2\msvc2019_64",
    "C:\Qt\5.15.2\mingw81_64"
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
    Write-Host "[WARNING] Qt directory not found automatically, please set QT_DIR env var" -ForegroundColor Yellow
    Write-Host "Example: `$env:QT_DIR = `"D:\tools\qt\6.11.1\mingw_64`"" -ForegroundColor Gray
    Write-Host ""
} else {
    Write-Host "[INFO] Using Qt directory: $qtDir" -ForegroundColor Cyan
    $env:PATH = "$qtDir\bin;$env:PATH"
}

# Create package directory
$packageDir = "ClipBridge_Qt"
if (Test-Path $packageDir) {
    Write-Host "[INFO] Cleaning old package directory..." -ForegroundColor Yellow
    Remove-Item -Path $packageDir -Recurse -Force
}
New-Item -Path $packageDir -ItemType Directory | Out-Null

Write-Host "[INFO] Copying files..." -ForegroundColor Cyan

# Copy main executable
Copy-Item $exePath -Destination "$packageDir\" -Force

# Copy config file
Copy-Item "config.json" -Destination "$packageDir\" -Force

# Copy docs
Copy-Item "README.md" -Destination "$packageDir\" -Force
Copy-Item "LICENSE" -Destination "$packageDir\" -Force

# Use windeployqt to package Qt dependencies
$exeDeployPath = Join-Path $packageDir "ClipBridge.exe"
if (Test-Path $exeDeployPath) {
    Write-Host "[INFO] Using windeployqt to package Qt dependencies..." -ForegroundColor Cyan

    $windeployqt = Get-Command "windeployqt.exe" -ErrorAction SilentlyContinue
    if (-not $windeployqt -and $qtDir) {
        $windeployqtPath = Join-Path $qtDir "bin\windeployqt.exe"
        if (Test-Path $windeployqtPath) {
            $windeployqt = $windeployqtPath
        }
    }

    if ($windeployqt) {
        & $windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw $exeDeployPath
    } else {
        Write-Host "[ERROR] Cannot find windeployqt!" -ForegroundColor Red
        Write-Host "Please make sure Qt is installed and QT_DIR is set correctly" -ForegroundColor Red
        Read-Host "Press Enter to exit"
        exit 1
    }
}

# Read version from CMakeLists.txt
function Get-VersionFromCMake {
    param([string]$cmakePath)
    if (Test-Path $cmakePath) {
        $content = Get-Content $cmakePath -Raw
        if ($content -match "project\(ClipBridge VERSION (\d+\.\d+\.\d+)") {
            return $matches[1]
        }
    }
    return $null
}

# Get version number
$version = $null

# 1. Try from CMakeLists.txt first
$cmakeVersion = Get-VersionFromCMake "CMakeLists.txt"
if ($cmakeVersion) {
    $version = $cmakeVersion
    Write-Host "[INFO] Read version from CMakeLists.txt: $version" -ForegroundColor Cyan
}

# 2. Try from git tag
if (-not $version) {
    try {
        $gitVersion = git describe --tags --abbrev=0 2>$null
        if ($gitVersion) {
            $version = $gitVersion
            Write-Host "[INFO] Read version from git tag: $version" -ForegroundColor Cyan
        }
    } catch {
        $version = $null
    }
}

# 3. Use date as fallback
if (-not $version) {
    $version = Get-Date -Format "yyyyMMdd"
    Write-Host "[INFO] Using date as version: $version" -ForegroundColor Yellow
}

# Create zip package
$zipName = "ClipBridge_Qt_$version.zip"
if (Test-Path $zipName) {
    Remove-Item $zipName -Force
}

Write-Host "[INFO] Creating zip package: $zipName..." -ForegroundColor Cyan
Compress-Archive -Path "$packageDir\*" -DestinationPath $zipName -Force

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "   Packaging complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "Output directory: $packageDir" -ForegroundColor White
Write-Host "Zip package: $zipName" -ForegroundColor White
Write-Host ""

# Open package directory
explorer $packageDir
