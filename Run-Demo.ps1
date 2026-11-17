param([switch]$Release)
$ErrorActionPreference = "Stop"
Push-Location $PSScriptRoot
try {
 $config = if ($Release) { "release" } else { "debug" }
 cmake --preset $config
 if ($LASTEXITCODE -ne 0) { throw "Configure failed" }
 cmake --build --preset $config --parallel 4
 if ($LASTEXITCODE -ne 0) { throw "Build failed" }
 & ".\build\$config\ThesisDemo.exe"
} finally { Pop-Location }
