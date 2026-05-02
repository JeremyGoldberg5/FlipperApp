#!/usr/bin/env pwsh
# Fetch and analyze the latest GitHub Actions workflow run logs
# Usage: .\fetch-logs.ps1 [-Analyze] [-ShowAll]

param(
    [switch]$Analyze,
    [switch]$ShowAll
)

$Repo = "JeremyGoldberg5/FlipperApp"
$LogsDir = ".logs"

# Create logs directory
if (-not (Test-Path $LogsDir)) {
    New-Item -ItemType Directory -Path $LogsDir | Out-Null
}

# Check if gh is installed
try {
    $ghVersion = gh --version 2>$null
} catch {
    Write-Host "ERROR: gh CLI not found. Install it first:" -ForegroundColor Red
    Write-Host "  Windows: winget install GitHub.cli"
    Write-Host "  Or: https://github.com/cli/cli/releases"
    exit 1
}

# Check if authenticated
$authStatus = gh auth status 2>&1
if ($authStatus -like "*not authenticated*") {
    Write-Host "ERROR: gh not authenticated. Run: gh auth login" -ForegroundColor Red
    exit 1
}

Write-Host "Fetching latest workflow runs for $Repo..." -ForegroundColor Cyan

# Get the latest workflow run
$latestRunJson = gh run list --repo $Repo --limit 1 --json databaseId,name,status,createdAt --jq '.[0]'
$latestRun = $latestRunJson | ConvertFrom-Json

$RunId = $latestRun.databaseId
$RunName = $latestRun.name
$RunStatus = $latestRun.status
$RunTime = $latestRun.createdAt

Write-Host "Latest run: $RunName (ID: $RunId)" -ForegroundColor Yellow
Write-Host "Status: $RunStatus" -ForegroundColor Yellow
Write-Host "Time: $RunTime" -ForegroundColor Yellow
Write-Host ""

# Fetch logs
Write-Host "Downloading logs..." -ForegroundColor Cyan
$LogFile = "$LogsDir/run-${RunId}.txt"
gh run logs $RunId --repo $Repo | Out-File -FilePath $LogFile -Encoding UTF8
Write-Host "✓ Logs saved to: $LogFile" -ForegroundColor Green
Write-Host ""

# Show summary
if (-not $ShowAll) {
    Write-Host "=== TAIL (Last 100 lines) ===" -ForegroundColor Cyan
    Get-Content $LogFile | Select-Object -Last 100
    Write-Host ""
}

# Analyze for errors if requested
if ($Analyze) {
    Write-Host "=== ERROR ANALYSIS ===" -ForegroundColor Cyan

    $logContent = Get-Content $LogFile -Raw

    # Check for cpp_ide.fap generation
    if ($logContent -match "FATAL: cpp_ide.fap was not built") {
        Write-Host "❌ cpp_ide.fap was not generated" -ForegroundColor Red
        Write-Host ""
        Write-Host "Checking for build failures..." -ForegroundColor Yellow
        $logContent -split "`n" | Where-Object { $_ -match "error|failed|fatal" -and $_ -notmatch "^CC|^LINK|^FAP|^APPMETA" } | Select-Object -First 20
    }

    if ($logContent -match "cpp_ide.fap found") {
        Write-Host "✅ cpp_ide.fap successfully built!" -ForegroundColor Green
        Write-Host ""
        $logContent -split "`n" | Where-Object { $_ -match "SUCCESS:" }
    }

    if ($logContent -match "✓ cpp_ide directory exists") {
        Write-Host "✅ cpp_ide was copied to applications/examples/" -ForegroundColor Green
    } else {
        Write-Host "❌ cpp_ide directory was not found" -ForegroundColor Red
    }

    if ($logContent -match "application.fam exists") {
        Write-Host "✅ application.fam file found" -ForegroundColor Green
    } else {
        Write-Host "❌ application.fam file not found" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "To view full logs: Get-Content $LogFile"
Write-Host "To analyze errors: .\fetch-logs.ps1 -Analyze"
Write-Host "To see everything: .\fetch-logs.ps1 -ShowAll"
