<#
.SYNOPSIS
    Compiles the mod's redscript files against the game's own script bundle, catching script errors without launching the game.

.PARAMETER GameDir
    Cyberpunk 2077 install folder. Defaults to the CYBERPUNK_2077_GAME_DIR environment variable.

.PARAMETER RedscriptVersion
    redscript CLI release used for the check. It is downloaded once into build/tools.
#>
param(
    [string]$GameDir = $env:CYBERPUNK_2077_GAME_DIR,
    [string]$RedscriptVersion = "0.5.31"
)

$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent

if (-not $GameDir) {
    throw "Pass -GameDir or set CYBERPUNK_2077_GAME_DIR to the Cyberpunk 2077 folder."
}

# Once redscript is installed in the game, final.redscripts holds compiled mods and the vanilla bundle moves to .bk.
$bundle = Join-Path $GameDir "r6\cache\final.redscripts"
if (Test-Path "$bundle.bk") {
    $bundle = "$bundle.bk"
}
if (-not (Test-Path $bundle)) {
    throw "Script bundle not found: $bundle"
}

$toolsDir = Join-Path $root "build\tools"
New-Item -ItemType Directory -Force $toolsDir | Out-Null

$cli = Join-Path $toolsDir "redscript-cli-$RedscriptVersion.exe"
if (-not (Test-Path $cli)) {
    $url = "https://github.com/jac3km4/redscript/releases/download/v$RedscriptVersion/redscript-cli.exe"
    Write-Host "Downloading $url"
    Invoke-WebRequest $url -OutFile $cli
}

# Mod Settings is optional at runtime, so both variants have to compile.
$scripts = Join-Path $root "scripts"
$variants = [ordered]@{
    "with Mod Settings"    = @($scripts, (Join-Path $PSScriptRoot "stubs"))
    "without Mod Settings" = @($scripts)
}

$output = Join-Path $toolsDir "check.redscripts"
foreach ($variant in $variants.GetEnumerator()) {
    $cliArgs = @("compile")
    foreach ($source in $variant.Value) {
        $cliArgs += @("-s", $source)
    }
    $cliArgs += @("-b", $bundle, "-o", $output)

    # The CLI exits with 0 even when compilation fails, so judge by its output instead.
    Remove-Item $output -ErrorAction SilentlyContinue
    $log = & $cli @cliArgs 2>&1 | ForEach-Object { "$_" }
    $log | Write-Host
    if (($log -match "ERROR|WARN") -or -not (Test-Path $output)) {
        throw "redscript compilation failed or warned ($($variant.Key))"
    }
    Write-Host "Scripts compile $($variant.Key) against $bundle"
}
