[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$BuildType = "Debug",
    [ValidateSet("Auto", "MSVC", "MinGW")]
    [string]$Toolchain = "Auto",
    [switch]$Build
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$buildDirectory = Join-Path $projectRoot "build-clangd"

function Get-FirstExistingPath {
    param([string[]]$Candidates)

    return $Candidates |
        Where-Object { $_ -and (Test-Path -LiteralPath $_) } |
        Select-Object -First 1
}

function Get-CommandPath {
    param([string]$Name)

    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }
    return $null
}

$mingwCxx = Get-FirstExistingPath @(
    (Get-CommandPath "g++.exe"),
    "C:\msys64\ucrt64\bin\g++.exe",
    "C:\msys64\mingw64\bin\g++.exe"
)

if ($Toolchain -eq "Auto") {
    if (Get-CommandPath "cl.exe") {
        $Toolchain = "MSVC"
    } elseif ($mingwCxx) {
        $Toolchain = "MinGW"
    } else {
        $Toolchain = "MSVC"
    }
}

$visualStudioPath = $null
$compilerArguments = @()
$expectedCxxCompiler = $null

if ($Toolchain -eq "MSVC") {
    $vswhere = Get-FirstExistingPath @(
        (Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"),
        (Join-Path $env:ProgramFiles "Microsoft Visual Studio\Installer\vswhere.exe")
    )

    if (-not $vswhere) {
        throw "MSVC requested, but vswhere.exe was not found. Install Visual Studio C++ tools or use -Toolchain MinGW."
    }

    $visualStudioPath = (& $vswhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath | Select-Object -First 1).Trim()

    if (-not $visualStudioPath) {
        throw "MSVC requested, but no Visual Studio installation with C++ tools was found."
    }

    $vsDevCmd = Join-Path $visualStudioPath "Common7\Tools\VsDevCmd.bat"
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

    $expectedCxxCompiler = Get-CommandPath "cl.exe"
    $sdkVersion = $env:WindowsSDKVersion.TrimEnd('\')
    $sdkBin = Join-Path $env:WindowsSdkDir "bin\$sdkVersion\x64"
    $resourceCompiler = (Join-Path $sdkBin "rc.exe").Replace('\', '/')
    $manifestTool = (Join-Path $sdkBin "mt.exe").Replace('\', '/')

    if (-not (Test-Path -LiteralPath $resourceCompiler) -or
        -not (Test-Path -LiteralPath $manifestTool)) {
        throw "Windows SDK x64 tools were not found under: $sdkBin"
    }

    $env:Path = "$sdkBin;$env:Path"
    $compilerArguments += "-DCMAKE_RC_COMPILER=$resourceCompiler"
    $compilerArguments += "-DCMAKE_MT=$manifestTool"
} else {
    if (-not $mingwCxx) {
        throw "MinGW requested, but g++.exe was not found in PATH or under C:\msys64."
    }

    $mingwBin = Split-Path -Parent $mingwCxx
    $mingwC = Join-Path $mingwBin "gcc.exe"
    if (-not (Test-Path -LiteralPath $mingwC)) {
        throw "MinGW gcc.exe was not found next to: $mingwCxx"
    }

    $env:Path = "$mingwBin;$env:Path"
    $expectedCxxCompiler = $mingwCxx
    $compilerArguments += "-DCMAKE_C_COMPILER=$($mingwC.Replace('\', '/'))"
    $compilerArguments += "-DCMAKE_CXX_COMPILER=$($mingwCxx.Replace('\', '/'))"

    $windres = Join-Path $mingwBin "windres.exe"
    if (Test-Path -LiteralPath $windres) {
        $compilerArguments += "-DCMAKE_RC_COMPILER=$($windres.Replace('\', '/'))"
    }
}

$cmake = Get-FirstExistingPath @(
    (Get-CommandPath "cmake.exe"),
    (Join-Path $env:ProgramFiles "CMake\bin\cmake.exe"),
    $(if ($visualStudioPath) {
        Join-Path $visualStudioPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    })
)
$ninja = Get-FirstExistingPath @(
    (Get-CommandPath "ninja.exe"),
    $(if ($visualStudioPath) {
        Join-Path $visualStudioPath "Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
    })
)

if (-not $cmake) {
    throw "CMake was not found."
}
if (-not $ninja) {
    throw "Ninja was not found."
}

$cachePath = Join-Path $buildDirectory "CMakeCache.txt"
if (Test-Path -LiteralPath $cachePath) {
    $cache = Get-Content -LiteralPath $cachePath
    $cachedGenerator = ($cache | Select-String '^CMAKE_GENERATOR:INTERNAL=(.+)$').Matches.Groups[1].Value
    $cachedCompiler = ($cache | Select-String '^CMAKE_CXX_COMPILER:FILEPATH=(.+)$').Matches.Groups[1].Value
    $expectedCompiler = $expectedCxxCompiler.Replace('\', '/')

    if ($cachedGenerator -ne "Ninja" -or
        $cachedCompiler.Replace('\', '/') -ine $expectedCompiler) {
        Write-Host "Toolchain changed; recreating $buildDirectory"
        & $cmake -E remove_directory $buildDirectory
        if ($LASTEXITCODE -ne 0) {
            throw "Failed to recreate the clangd build directory."
        }
    }
}

$configureArguments = @(
    "-S", $projectRoot,
    "-B", $buildDirectory,
    "-G", "Ninja",
    "-DCMAKE_MAKE_PROGRAM=$ninja",
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
) + $compilerArguments

& $cmake @configureArguments

if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed."
}

if ($Build) {
    & $cmake --build $buildDirectory --parallel
    if ($LASTEXITCODE -ne 0) {
        throw "The C++ build failed."
    }
}

Write-Host "Toolchain: $Toolchain ($expectedCxxCompiler)"
Write-Host "clangd compilation database: $buildDirectory\compile_commands.json"
