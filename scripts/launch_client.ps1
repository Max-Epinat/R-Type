# R-Type Client Launcher for Windows
# Launches a single R-Type client instance

param(
    [Parameter(Mandatory=$false)]
    [string]$ServerHost = '127.0.0.1',
    
    [Parameter(Mandatory=$false)]
    [int]$Port = 5000,
    
    [Parameter(Mandatory=$false)]
    [ValidateSet('Debug', 'Release')]
    [string]$BuildType = 'Release'
)

$ErrorActionPreference = "Stop"

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  R-Type Client Launcher" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# Navigate to project root
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptDir
Push-Location $projectRoot

# Check if client binary exists
$clientPath = "bin\$BuildType\rtype_client.exe"

if (-not (Test-Path $clientPath)) {
    Write-Host "[ERROR] Client executable not found at: $clientPath" -ForegroundColor Red
    Write-Host "Please build the project first:" -ForegroundColor Yellow
    Write-Host "  cd scripts" -ForegroundColor White
    Write-Host "  .\build.ps1 -BuildType $BuildType" -ForegroundColor White
    Pop-Location
    exit 1
}

Write-Host "Launching R-Type Client..." -ForegroundColor Yellow
Write-Host "  Build Type: $BuildType" -ForegroundColor Gray
Write-Host "  Server:     $ServerHost`:$Port" -ForegroundColor Gray
Write-Host ""

# Launch client with working directory set to project root (for config/assets)
try {
    Start-Process -FilePath $clientPath -ArgumentList $ServerHost, $Port -WorkingDirectory $projectRoot -Wait
    Write-Host ""
    Write-Host "Client closed." -ForegroundColor Green
} catch {
    Write-Host "[ERROR] Failed to launch client: $_" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location
Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  Thanks for playing!" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
