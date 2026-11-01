$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$setup = 'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path -LiteralPath $setup)) {
    throw 'MSVC setup was not found. Run the equivalent cl command from an installed Developer Command Prompt.'
}
Push-Location $projectRoot
try {
    New-Item -ItemType Directory -Force submissions/game | Out-Null
    $sources = 'src\main.c src\platform.c src\net.c src\protocol.c src\content.c src\game.c src\server.c src\client.c tests\test_game.c tests\test_protocol.c third_party\cJSON.c'
    $command = '"' + $setup + '" >nul && cl /nologo /std:c17 /W4 /WX /D_CRT_SECURE_NO_WARNINGS /DCJSON_NESTING_LIMIT=8 ' + $sources + ' /Fosubmissions\game\ /Fesubmissions\game\2r1e.exe ws2_32.lib'
    & cmd /d /s /c $command
    if ($LASTEXITCODE -ne 0) { throw 'Windows build failed.' }
    & ./submissions/game/2r1e.exe --self-test
    if ($LASTEXITCODE -ne 0) { throw 'Windows checks failed.' }
    Write-Output 'Windows executable: submissions/game/2r1e.exe'
} finally { Pop-Location }
