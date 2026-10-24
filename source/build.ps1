param(
    [string]$QtKit = "C:\Users\20347\Documents\Codex\2026-09-28\x20\work\qt-sdk\6.8.3\mingw_64",
    [string]$QtTools = "E:\QT\Tools"
)

# Builds outside the source tree and replaces ..\runtime\ClearOSD.exe.
# The runtime folder already holds the matching Qt 6.8.3 DLLs.
$ErrorActionPreference = "Stop"

$compilerBin = Join-Path (Split-Path (Split-Path $QtKit)) "Tools\mingw1310_64\bin"
$cmake = Join-Path $QtTools "CMake_64\bin\cmake.exe"
$ninja = Join-Path $QtTools "Ninja\ninja.exe"
$buildDirectory = Join-Path $env:TEMP "ClearOSD-build"
$runtimeDirectory = Join-Path $PSScriptRoot "..\runtime"

$env:Path = "$compilerBin;$QtKit\bin;$($env:Path)"

& $cmake -S $PSScriptRoot -B $buildDirectory -G Ninja `
    -DCMAKE_MAKE_PROGRAM="$($ninja.Replace('\', '/'))" `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_PREFIX_PATH="$($QtKit.Replace('\', '/'))" `
    -DCMAKE_CXX_COMPILER="$((Join-Path $compilerBin 'g++.exe').Replace('\', '/'))" `
    -DCMAKE_RC_COMPILER="$((Join-Path $compilerBin 'windres.exe').Replace('\', '/'))"
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed ($LASTEXITCODE)" }

& $cmake --build $buildDirectory --parallel
if ($LASTEXITCODE -ne 0) { throw "CMake build failed ($LASTEXITCODE)" }

if (Get-Process ClearOSD -ErrorAction SilentlyContinue) { throw "Quit ClearOSD before replacing the runtime executable" }
Copy-Item -Force (Join-Path $buildDirectory "ClearOSD.exe") $runtimeDirectory
Write-Host "Build complete: $(Resolve-Path $runtimeDirectory)\ClearOSD.exe"
