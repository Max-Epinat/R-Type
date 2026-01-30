# R-Type Build Script for Windows
# This script builds the R-Type project using Conan and CMake on Windows
# ⚠️ IMPORTANT: Run this script from the project ROOT directory, not from the scripts folder

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet('Debug', 'Release')]
    [string]$BuildType = 'Release'
)

$ErrorActionPreference = "Stop"

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  R-Type Build Script for Windows" -ForegroundColor Cyan
Write-Host "  Build Type: $BuildType" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

if (-not (Test-Path "CMakeLists.txt")) {
    Write-Host "[ERROR] CMakeLists.txt not found!" -ForegroundColor Red
    Write-Host "        This script must be run from the project ROOT directory." -ForegroundColor Red
    Write-Host "   Example: cd r-type && .\build.ps1" -ForegroundColor Yellow
    exit 1
}

Write-Host "[1/5] Checking prerequisites..." -ForegroundColor Yellow

try {
    $cmakeOutput = & cmake --version 2>&1 | Out-String
    if ($cmakeOutput -match 'version (\d+\.\d+\.\d+)') {
        Write-Host "  CMake found: $($Matches[1])" -ForegroundColor Green
    } else {
        throw "CMake version not detected"
    }
} catch {
    Write-Host "  CMake not found. Please install CMake 3.28+" -ForegroundColor Red
    Write-Host "    Download from: https://cmake.org/download/" -ForegroundColor Yellow
    exit 1
}

$conanCmd = "conan"
try {
    $conanOutput = & conan --version 2>&1 | Out-String
    if ($conanOutput -match 'version (\d+\.\d+\.\d+)') {
        Write-Host "  Conan found: $($Matches[1])" -ForegroundColor Green
    } else {
        throw "Conan command failed"
    }
} catch {
    Write-Host "  Conan not found. Please install Conan 2.0+" -ForegroundColor Red
    Write-Host "    Install with: pip install conan" -ForegroundColor Yellow
    Write-Host "    Or use: py -m pip install conan" -ForegroundColor Yellow
    exit 1
}

$vsVersion = & where.exe cl.exe 2>$null
if ($vsVersion) {
    Write-Host "  Visual C++ compiler found" -ForegroundColor Green
} else {
    Write-Host "  Visual C++ not in PATH. Make sure to run from Developer Command Prompt" -ForegroundColor Yellow
    Write-Host "    or Visual Studio Build Tools are installed" -ForegroundColor Yellow
}

$profilePath = Join-Path $env:USERPROFILE ".conan2\profiles\default"
if (-not (Test-Path $profilePath)) {
    Write-Host "  Creating Conan default profile..." -ForegroundColor Yellow
    & $conanCmd profile detect --force
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  Failed to create Conan profile" -ForegroundColor Red
        exit 1
    }
    Write-Host "  Conan profile created successfully" -ForegroundColor Green
}

Write-Host ""

# Install dependencies for BOTH Debug and Release to ensure both work
# This prevents the common issue where debug builds fail due to missing debug DLLs
Write-Host "[2/6] Installing Conan dependencies for Debug and Release..." -ForegroundColor Yellow
Write-Host "  (Installing both configurations ensures debug DLLs are available)" -ForegroundColor Gray

$buildTypes = @('Debug', 'Release')
foreach ($type in $buildTypes) {
    $tempBuildDir = "build\$type"
    Write-Host "  Installing $type dependencies..." -ForegroundColor Cyan
    New-Item -ItemType Directory -Force -Path $tempBuildDir | Out-Null
    Push-Location $tempBuildDir
    try {
        $conanArgs = @(
            "install", "..\\..",
            "--output-folder=.",
            "--build=missing",
            "-s", "build_type=$type",
            "-s", "compiler.cppstd=20",
            "-s", "compiler.runtime=dynamic"
        )
        & $conanCmd @conanArgs
        if ($LASTEXITCODE -ne 0) {
            Pop-Location
            throw "Conan install failed for $type"
        }
        Write-Host "    $type dependencies installed [OK]" -ForegroundColor Green
    } catch {
        Write-Host "    Failed to install $type dependencies" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    Pop-Location
}

Write-Host ""
$buildDir = "build\$BuildType"
Write-Host "[3/6] Preparing build directory: $buildDir" -ForegroundColor Yellow

# Verify the toolchain file was created during dependency installation
$toolchainFile = "build\$BuildType\build\generators\conan_toolchain.cmake"
if (-not (Test-Path $toolchainFile)) {
    Write-Host "  ERROR: conan_toolchain.cmake not found!" -ForegroundColor Red
    Write-Host "  Expected at: $toolchainFile" -ForegroundColor Yellow
    Write-Host "  Dependencies may not have installed correctly for $BuildType" -ForegroundColor Red
    exit 1
}
Write-Host "  Toolchain file verified" -ForegroundColor Green

Write-Host ""
Write-Host "[4/6] Configuring project with CMake..." -ForegroundColor Yellow
Push-Location $buildDir
try {
    $toolchainFile = Join-Path (Get-Location) "build\generators\conan_toolchain.cmake"
    $cmakeArgs = @(
        "..\\..",
        "-DCMAKE_TOOLCHAIN_FILE=$toolchainFile",
        "-DCMAKE_BUILD_TYPE=$BuildType"
    )
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    Write-Host "  Project configured successfully" -ForegroundColor Green
} catch {
    Write-Host "  Failed to configure project" -ForegroundColor Red
    Pop-Location
    exit 1
}
Write-Host ""

Write-Host "[5/6] Building project..." -ForegroundColor Yellow
try {
    & cmake --build . --config $BuildType
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    Write-Host "  Build completed successfully" -ForegroundColor Green
} catch {
    Write-Host "  Build failed" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location
Write-Host ""
Write-Host "[6/6] Verifying build output..." -ForegroundColor Yellow

$binDir = "bin"
if (Test-Path $binDir) {
    $serverExe = Get-ChildItem -Path $binDir -Filter "rtype_server.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
    $clientExe = Get-ChildItem -Path $binDir -Filter "rtype_client.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
    
    if ($serverExe -and $clientExe) {
        Write-Host "  Server: $($serverExe.FullName)" -ForegroundColor Green
        Write-Host "  Client: $($clientExe.FullName)" -ForegroundColor Green
    } else {
        Write-Host "  Warning: Executables not found in expected location" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  Build Complete!" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "[SUCCESS] Both Debug and Release dependencies installed" -ForegroundColor Green
Write-Host "          Debug builds will now work correctly!" -ForegroundColor Green
Write-Host ""
Write-Host "Executables are located in:" -ForegroundColor Yellow
Write-Host '  bin\rtype_server.exe' -ForegroundColor White
Write-Host '  bin\rtype_client.exe' -ForegroundColor White
Write-Host ""
Write-Host 'To run the game:' -ForegroundColor Yellow
Write-Host '  cd scripts' -ForegroundColor White
Write-Host '  .\launch.ps1' -ForegroundColor White
Write-Host ""
