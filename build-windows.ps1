$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

foreach ($tool in @('cmake', 'ctest', 'git')) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "Install $tool and Visual Studio 2022 Desktop development with C++ first."
    }
}

cmake --preset windows-x64
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
cmake --build --preset windows-x64 --parallel 3
if ($LASTEXITCODE -ne 0) { throw 'Windows x64 build failed.' }
ctest --preset windows-x64
if ($LASTEXITCODE -ne 0) { throw 'Tests failed; no package was created.' }

$artifacts = Join-Path $PSScriptRoot 'build/windows-x64/UnpluggedStudy_artefacts/Release'

function Assert-WindowsX64Binary([string] $Path, [bool] $ExpectDll) {
    $bytes = [System.IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -lt 64 -or [BitConverter]::ToUInt16($bytes, 0) -ne 0x5A4D) {
        throw "Not a Windows binary: $Path"
    }
    $pe = [BitConverter]::ToInt32($bytes, 0x3C)
    if ($pe -lt 0 -or $pe -gt ($bytes.Length - 24)) { throw "Invalid PE header: $Path" }
    if ([BitConverter]::ToUInt32($bytes, $pe) -ne 0x00004550 -or
        [BitConverter]::ToUInt16($bytes, $pe + 4) -ne 0x8664) {
        throw "Binary is not Windows AMD64: $Path"
    }
    $isDll = ([BitConverter]::ToUInt16($bytes, $pe + 22) -band 0x2000) -ne 0
    if ($isDll -ne $ExpectDll) { throw "Unexpected PE binary type: $Path" }
    Write-Host "Verified Windows x64 binary: $Path"
}

$vstBinary = @(Get-ChildItem -Path (Join-Path $artifacts 'VST3') -Recurse -File -Filter '*.vst3')
$standalone = @(Get-ChildItem -Path (Join-Path $artifacts 'Standalone') -File -Filter '*.exe')
if ($vstBinary.Count -ne 1 -or $standalone.Count -ne 1) { throw 'Expected one VST3 module and one standalone executable.' }
Assert-WindowsX64Binary $vstBinary[0].FullName $true
Assert-WindowsX64Binary $standalone[0].FullName $false

$dist = Join-Path $PSScriptRoot 'dist'
New-Item -ItemType Directory -Force -Path $dist | Out-Null
$zip = Join-Path $dist 'Unplugged-Study-Windows-x64.zip'
Compress-Archive -Path (Join-Path $artifacts 'VST3'), (Join-Path $artifacts 'Standalone') -DestinationPath $zip -Force
$digest = (Get-FileHash -Path $zip -Algorithm SHA256).Hash.ToLowerInvariant()
[System.IO.File]::WriteAllText("$zip.sha256", "$digest  Unplugged-Study-Windows-x64.zip`n", [System.Text.UTF8Encoding]::new($false))
Write-Host "Build and tests completed. Package: $zip"
