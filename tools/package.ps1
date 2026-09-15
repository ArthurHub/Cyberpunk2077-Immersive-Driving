<#
.SYNOPSIS
    Builds the plugin and creates dist/ImmersiveDriving-<version>.zip, laid out to extract into the game folder.

.PARAMETER Config
    CMake build configuration.
#>
param(
    [string]$Config = "Release"
)

$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent
$build = Join-Path $root "build"

if (-not (Test-Path (Join-Path $build "CMakeCache.txt"))) {
    cmake --preset default
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }
}

cmake --build $build --config $Config
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

ctest --test-dir $build -C $Config --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "Tests failed" }

$version = (Select-String -Path (Join-Path $root "CMakeLists.txt") -Pattern '^\s*VERSION\s+([0-9.]+)').Matches[0].Groups[1].Value
$dist = Join-Path $root "dist"
$staging = Join-Path $dist "staging"
if (Test-Path $staging) {
    Remove-Item -Recurse -Force $staging
}

cmake --install $build --config $Config --prefix $staging
if ($LASTEXITCODE -ne 0) { throw "Install to staging failed" }

$archive = Join-Path $dist "ImmersiveDriving-$version.zip"
Compress-Archive -Path (Join-Path $staging "*") -DestinationPath $archive -Force
Write-Host "Created $archive"
