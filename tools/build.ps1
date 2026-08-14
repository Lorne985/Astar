[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$BuildType = "Debug"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $root "build"
$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"

if (-not (Test-Path $vswhere)) {
    throw "Visual Studio Build Tools was not found."
}

$vsPath = (& $vswhere -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath).Trim()

if (-not $vsPath) {
    throw "MSVC C++ tools were not found."
}

$vsDevCmd = Join-Path $vsPath "Common7\Tools\VsDevCmd.bat"
$environment = & $env:ComSpec /d /c "call `"$vsDevCmd`" -no_logo -arch=x64 -host_arch=x64 >nul && set"
foreach ($line in $environment) {
    if ($line -match '^([^=]+)=(.*)$') {
        Set-Item "Env:$($matches[1])" $matches[2]
    }
}

$cmake = Join-Path $vsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$ninja = Join-Path $vsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"

& $cmake -S $root -B $buildDir -G Ninja `
    "-DCMAKE_MAKE_PROGRAM=$ninja" `
    "-DCMAKE_BUILD_TYPE=$BuildType" `
    "-DCMAKE_C_COMPILER=cl.exe" `
    "-DCMAKE_CXX_COMPILER=cl.exe"

if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed."
}

& $cmake --build $buildDir --parallel
if ($LASTEXITCODE -ne 0) {
    throw "Build failed."
}

Write-Host "Built with MSVC: $buildDir\astar_visualizer.exe"
