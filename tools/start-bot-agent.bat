@echo off
setlocal

set "MASTER=%~1"
set "TOKEN=%~2"
set "NAME=%~3"
set "MAX_BOTS=%~4"
if "%MASTER%"=="" (
    echo usage: start-bot-agent.bat ws://master-host:8089 token [name] [max-bots]
    exit /b 1
)
if "%NAME%"=="" set "NAME=%COMPUTERNAME%"
if "%MAX_BOTS%"=="" set "MAX_BOTS=500"

for %%I in ("%~dp0..\server\bot") do set "BOT_DIR=%%~fI"
set "BOT_EXE=%BOT_DIR%\Debug\bot.exe"
if not exist "%BOT_EXE%" (
    echo bot.exe not found: %BOT_EXE%
    exit /b 1
)

set "TOKEN_ARG="
if not "%TOKEN%"=="" set "TOKEN_ARG=--token %TOKEN%"

start "bot %NAME%" /D "%BOT_DIR%" "%BOT_EXE%" --role agent --master %MASTER% --name %NAME% --max-bots %MAX_BOTS% %TOKEN_ARG%
exit /b 0
