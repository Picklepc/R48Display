param(
  [string]$Environment = "waveshare_esp32_s3_touch_lcd_1_85",
  [string]$Version = "",
  [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$BoardConfig = Join-Path $Root "include\BoardConfig.h"
$FirmwareDir = Join-Path $Root "firmware"
$BuildDir = Join-Path $Root ".pio\build\$Environment"

if (-not $Version) {
  $ConfigText = Get-Content -Raw $BoardConfig
  if ($ConfigText -notmatch 'FIRMWARE_VERSION\s*=\s*"([^"]+)"') {
    throw "Could not read FIRMWARE_VERSION from $BoardConfig"
  }
  $Version = $Matches[1]
}

function Resolve-Tool([string]$PreferredPath, [string]$CommandName) {
  if ($PreferredPath -and (Test-Path $PreferredPath)) {
    return $PreferredPath
  }
  $Command = Get-Command $CommandName -ErrorAction SilentlyContinue
  if ($Command) {
    return $Command.Source
  }
  throw "Could not find $CommandName"
}

$Pio = Resolve-Tool (Join-Path $env:USERPROFILE ".platformio\penv\Scripts\platformio.exe") "platformio"
$Python = Resolve-Tool (Join-Path $env:USERPROFILE ".platformio\penv\Scripts\python.exe") "python"
$Esptool = Join-Path $env:USERPROFILE ".platformio\packages\tool-esptoolpy\esptool.py"
$BootApp0 = Join-Path $env:USERPROFILE ".platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin"

if (-not $SkipBuild) {
  & $Pio run -e $Environment
}

$Bootloader = Join-Path $BuildDir "bootloader.bin"
$Partitions = Join-Path $BuildDir "partitions.bin"
$App = Join-Path $BuildDir "firmware.bin"

foreach ($Path in @($Esptool, $BootApp0, $Bootloader, $Partitions, $App)) {
  if (-not (Test-Path $Path)) {
    throw "Required firmware artifact missing: $Path"
  }
}

New-Item -ItemType Directory -Force -Path $FirmwareDir | Out-Null

$AppOut = Join-Path $FirmwareDir "R48Display-v$Version.bin"
$MergedOut = Join-Path $FirmwareDir "R48Display-v$Version-merged.bin"
$HashOut = Join-Path $FirmwareDir "R48Display-v$Version.sha256"

Copy-Item -Force $App $AppOut

& $Python $Esptool --chip esp32s3 merge_bin `
  -o $MergedOut `
  --flash_mode qio `
  --flash_freq 80m `
  --flash_size 16MB `
  0x0 $Bootloader `
  0x8000 $Partitions `
  0xe000 $BootApp0 `
  0x10000 $App

Get-FileHash -Algorithm SHA256 $AppOut, $MergedOut |
  ForEach-Object { "$($_.Hash.ToLower())  $(Split-Path $_.Path -Leaf)" } |
  Set-Content -Encoding ascii $HashOut

Write-Host "Packaged:"
Write-Host "  $AppOut"
Write-Host "  $MergedOut"
Write-Host "  $HashOut"
