@echo off
setlocal

set "AGENTS=%~1"
if "%AGENTS%"=="" set "AGENTS=3"
set "LISTEN=%~2"
if "%LISTEN%"=="" set "LISTEN=127.0.0.1:8089"
set "TOKEN=%~3"

for %%I in ("%~dp0..\server\bot") do set "BOT_DIR=%%~fI"
set "BOT_EXE=%BOT_DIR%\Debug\bot.exe"
if not exist "%BOT_EXE%" (
    echo bot.exe not found: %BOT_EXE%
    exit /b 1
)

powershell -NoProfile -Command "$c = New-Object Net.Sockets.TcpClient; try { $c.Connect('127.0.0.1', 3001); exit 0 } catch { exit 1 } finally { $c.Dispose() }"
if errorlevel 1 (
    echo gateway 127.0.0.1:3001 is not listening. Run local-scripts\start-bot-test.bat first.
    exit /b 1
)

set "TOKEN_ARG="
if not "%TOKEN%"=="" set "TOKEN_ARG=--token %TOKEN%"

echo Starting master on %LISTEN%
start "bot master" /D "%BOT_DIR%" "%BOT_EXE%" --role master --listen %LISTEN% %TOKEN_ARG%

for /f "tokens=1,2 delims=:" %%A in ("%LISTEN%") do (
    set "HOST=%%A"
    set "PORT=%%B"
)
if "%HOST%"=="0.0.0.0" set "HOST=127.0.0.1"
powershell -NoProfile -Command "for ($i = 0; $i -lt 60; $i++) { $c = New-Object Net.Sockets.TcpClient; try { $c.Connect('%HOST%', %PORT%); exit 0 } catch { Start-Sleep -Milliseconds 500 } finally { $c.Dispose() } }; exit 1"
if errorlevel 1 (
    echo master did not open %LISTEN%
    exit /b 1
)

for /L %%N in (1,1,%AGENTS%) do (
    echo Starting agent-%%N
    start "bot agent-%%N" /D "%BOT_DIR%" "%BOT_EXE%" --role agent --master ws://%HOST%:%PORT% --name agent-%%N %TOKEN_ARG%
)

start "" "http://%HOST%:%PORT%"
exit /b 0
