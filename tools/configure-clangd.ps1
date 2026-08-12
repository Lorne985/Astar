[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$BuildType = "Debug",
    [switch]$Build
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$buildDirectory = Join-Path $projectRoot "build-clangd"

$vswhereCandidates = @(
    (Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"),
    (Join-Path $env:ProgramFiles "Microsoft Visual Studio\Installer\vswhere.exe")
)
$vswhere = $vswhereCandidates |
    Where-Object { $_ -and (Test-Path -LiteralPath $_) } |
    Select-Object -First 1

if (-not $vswhere) {
    throw "vswhere.exe was not found. Install Visual Studio with Desktop development with C++."
}

$visualStudioPath = (& $vswhere -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath | Select-Object -First 1).Trim()

if (-not $visualStudioPath) {
    throw "A Visual Studio installation with the MSVC C++ tools was not found."
}

$vsDevCmd = Join-Path $visualStudioPath "Common7\Tools\VsDevCmd.bat"
if (-not (Test-Path -LiteralPath $vsDevCmd)) {
    throw "Visual Studio developer environment script was not found: $vsDevCmd"
}

$developerCommand = "call `"$vsDevCmd`" -no_logo -arch=x64 -host_arch=x64 >nul && set"
$developerEnvironment = & $env:ComSpec /d /c $developerCommand
if ($LASTEXITCODE -ne 0) {
    throw "Failed to initialize the Visual Studio x64 developer environment."
}

foreach ($line in $developerEnvironment) {
    if ($line -match '^([^=]+)=(.*)$') {
        Set-Item -Path "Env:$($matches[1])" -Value $matches[2]
    }
}

$cmakeCandidates = @(
    (Join-Path $env:ProgramFiles "CMake\bin\cmake.exe"),
    (Join-Path $visualStudioPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe")
)
$cmake = $cmakeCandidates |
    Where-Object { Test-Path -LiteralPath $_ } |
    Select-Object -First 1

$ninjaCandidates = @(
    (Join-Path $visualStudioPath "Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"),
    ((Get-Command ninja.exe -ErrorAction SilentlyContinue).Source)
)
$ninja = $ninjaCandidates |
    Where-Object { $_ -and (Test-Path -LiteralPath $_) } |
    Select-Object -First 1

if (-not $cmake) {
    throw "CMake was not found. Install CMake or the Visual Studio CMake tools."
}
if (-not $ninja) {
    throw "Ninja was not found. Install Ninja or the Visual Studio CMake tools."
}

$sdkVersion = $env:WindowsSDKVersion.TrimEnd('\')
$sdkBin = Join-Path $env:WindowsSdkDir "bin\$sdkVersion\x64"
$resourceCompiler = Join-Path $sdkBin "rc.exe"
$manifestTool = Join-Path $sdkBin "mt.exe"

if (-not (Test-Path -LiteralPath $resourceCompiler) -or
    -not (Test-Path -LiteralPath $manifestTool)) {
    throw "Windows SDK x64 tools were not found under: $sdkBin"
}

$env:Path = "$sdkBin;$env:Path"

& $cmake `
    -S $projectRoot `
    -B $buildDirectory `
    -G Ninja `
    "-DCMAKE_MAKE_PROGRAM=$ninja" `
    "-DCMAKE_BUILD_TYPE=$BuildType" `
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON" `
    "-DCMAKE_RC_COMPILER=$resourceCompiler" `
    "-DCMAKE_MT=$manifestTool"

if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed."
}

if ($Build) {
    & $cmake --build $buildDirectory --parallel
    if ($LASTEXITCODE -ne 0) {
        throw "The C++ build failed."
    }
}

Write-Host "clangd compilation database: $buildDirectory\compile_commands.json"
