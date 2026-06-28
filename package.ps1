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
$sourceDir = ""
if (Test-Path "build\ClipBridge.exe") {
    $sourceDir = "build"
    Write-Host "[INFO] Detected MinGW build" -ForegroundColor Green
} elseif (Test-Path "build\Release\ClipBridge.exe") {
    $sourceDir = "build\Release"
    Write-Host "[INFO] Detected MSVC build" -ForegroundColor Green
} else {
    Write-Host "[ERROR] ClipBridge.exe not found! Please compile first." -ForegroundColor Red
    Write-Host "Looking for:" -ForegroundColor Yellow
    Write-Host "  - build\ClipBridge.exe (MinGW)"
    Write-Host "  - build\Release\ClipBridge.exe (MSVC)"
    Read-Host "Press Enter to exit"
    exit 1
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

# Create package directory
$packageDir = "ClipBridge_Qt"
if (Test-Path $packageDir) {
    Write-Host "[INFO] Cleaning old package directory..." -ForegroundColor Yellow
    Remove-Item -Path $packageDir -Recurse -Force
}
New-Item -Path $packageDir -ItemType Directory | Out-Null

Write-Host "[INFO] Copying files from $sourceDir..." -ForegroundColor Cyan

# Copy everything from build directory (already has DLLs from windeployqt)
Copy-Item "$sourceDir\*" -Destination "$packageDir\" -Recurse -Force

# Copy additional files
Copy-Item "README.md" -Destination "$packageDir\" -Force
Copy-Item "config.json" -Destination "$packageDir\" -Force
if (Test-Path "LICENSE") {
    Copy-Item "LICENSE" -Destination "$packageDir\" -Force
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
