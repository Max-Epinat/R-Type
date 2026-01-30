# R-Type Launch Script for Windows
# Launches the R-Type server and client

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet('Debug', 'Release')]
    [string]$BuildType = 'Debug'
)

$ErrorActionPreference = "Stop"

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  R-Type Launcher" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# Navigate to project root
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptDir
Push-Location $projectRoot

# Check if binaries exist
$serverPath = "bin\$BuildType\rtype_server.exe"
$clientPath = "bin\$BuildType\rtype_client.exe"

if (-not (Test-Path $serverPath)) {
    Write-Host "Server executable not found at: $serverPath" -ForegroundColor Red
    Write-Host "Please build the project first:" -ForegroundColor Yellow
    Write-Host "  cd scripts" -ForegroundColor White
    Write-Host "  .\build.ps1 -BuildType $BuildType" -ForegroundColor White
    Pop-Location
    exit 1
}

if (-not (Test-Path $clientPath)) {
    Write-Host "Client executable not found at: $clientPath" -ForegroundColor Red
    Write-Host "Please build the project first:" -ForegroundColor Yellow
    Write-Host "  cd scripts" -ForegroundColor White
    Write-Host "  .\build.ps1 -BuildType $BuildType" -ForegroundColor White
    Pop-Location
    exit 1
}

# Check if config file exists
if (-not (Test-Path "config\game.ini")) {
    Write-Host "Warning: config/game.ini not found. Using default settings." -ForegroundColor Yellow
}

# Start server with working directory set to project root (for config/assets)
Write-Host "Starting R-Type Server..." -ForegroundColor Yellow
$serverProcess = Start-Process -FilePath $serverPath -WorkingDirectory $projectRoot -PassThru -WindowStyle Normal
Write-Host "  Server started (PID: $($serverProcess.Id))" -ForegroundColor Green
Write-Host ""

# Wait for server to initialize
Write-Host "Waiting for server to initialize..." -ForegroundColor Gray
Start-Sleep -Seconds 2

# Check if server is still running
if ($serverProcess.HasExited) {
    Write-Host "Server failed to start. Check logs above." -ForegroundColor Red
    Pop-Location
    exit 1
}

# Start client with working directory set to project root (for config/assets)
Write-Host "Starting R-Type Client..." -ForegroundColor Yellow
try {
    $clientProcess = Start-Process -FilePath $clientPath -WorkingDirectory $projectRoot -PassThru -Wait
    Write-Host ""
    Write-Host "Client closed." -ForegroundColor Gray
} finally {
    # Clean up server
    Write-Host "Stopping server..." -ForegroundColor Yellow
    Stop-Process -Id $serverProcess.Id -Force -ErrorAction SilentlyContinue
    Write-Host "  Server stopped." -ForegroundColor Green
}

Pop-Location
Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  Thanks for playing!" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
