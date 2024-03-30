$buildPath = Split-Path -Parent $MyInvocation.MyCommand.Definition

$buildDir = Join-Path -Path $buildPath -ChildPath "Build"
if (-not (Test-Path $buildDir)) {
    New-Item -Path $buildDir -ItemType Directory
}

Set-Location $buildDir

& conan install ../Engine/ --build=missing -s cppstd=23 -s build_type=Debug

& cmake .. -G "Visual Studio 17"

Read-Host "Press Enter to continue..."
