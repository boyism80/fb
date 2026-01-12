@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

pushd ..

set "GIT_URL=https://github.com/git-for-windows/git/releases/download/v2.49.0.windows.1/Git-2.49.0-64-bit.exe"
set "CMAKE_URL=https://github.com/Kitware/CMake/releases/download/v4.0.2/cmake-4.0.2-windows-x86_64.msi"

set "GIT_INSTALLER=%TEMP%\Git-2.49.0-64-bit.exe"
set "CMAKE_INSTALLER=%TEMP%\cmake-4.0.2-windows-x86_64.msi"

git --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Git is not installed.
    echo Please download it from:
    echo     https://git-scm.com/downloads/win
    goto :ErrorExit
)

cmake --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake is not installed.
    echo Please download it from:
    echo     https://cmake.org/download/
    goto :ErrorExit
)

:: Check for .NET Core 8.0
echo [INFO] Checking for .NET Core 8.0...
dotnet --list-runtimes | find "Microsoft.NETCore.App 8.0" >nul
if errorlevel 1 (
    echo [ERROR] .NET Core 8.0 is not installed.
    echo Please download and install .NET Core 8.0 from:
    echo https://dotnet.microsoft.com/download/dotnet/8.0
    goto :ErrorExit
)
echo [INFO] .NET Core 8.0 is installed.

:: 1) Retrieve external IP using OpenDNS
set "IP="
for /f "tokens=2 delims=: " %%a in (
    'nslookup myip.opendns.com. resolver1.opendns.com 2^>nul ^| find "Address:"'
) do (
    set "IP=%%a"
)
if "%IP%"=="" (
    echo [ERROR] Unable to retrieve external IP. Check network or DNS settings.
    goto :ErrorExit
)
set /p EXTERNAL_IP=Enter External IP [!IP!]: 
if "%EXTERNAL_IP%"=="" set "EXTERNAL_IP=%IP%"
echo [INFO] External IP: !IP!

:: 2) Prompt for MySQL connection info (defaults provided)
set "MYSQL_SERVER="
set /p MYSQL_SERVER=Enter MySQL Server IP [127.0.0.1]: 
if "%MYSQL_SERVER%"=="" set "MYSQL_SERVER=127.0.0.1"
set "MYSQL_PORT="
set /p MYSQL_PORT=Enter MySQL Port [3306]: 
if "%MYSQL_PORT%"=="" set "MYSQL_PORT=3306"
set "MYSQL_USER="
set /p MYSQL_USER=Enter MySQL User ID [root]: 
if "%MYSQL_USER%"=="" set "MYSQL_USER=root"
set "MYSQL_PWD="
set /p MYSQL_PWD=Enter MySQL Password [admin]: 
if "%MYSQL_PWD%"=="" set "MYSQL_PWD=admin"
set "MYSQL_DB="
set /p MYSQL_DB=Enter MySQL Database Name [fb]: 
if "%MYSQL_DB%"=="" set "MYSQL_DB=fb"
echo [INFO] MySQL Connection: !MYSQL_SERVER!:!MYSQL_PORT! user=!MYSQL_USER!

:: 3) Prompt for Redis and RabbitMQ info (defaults provided)
set "REDIS_HOST="
set /p REDIS_HOST=Enter Redis Host [127.0.0.1]: 
if "%REDIS_HOST%"=="" set "REDIS_HOST=127.0.0.1"
set "REDIS_PORT="
set /p REDIS_PORT=Enter Redis Port [6379]: 
if "%REDIS_PORT%"=="" set "REDIS_PORT=6379"
echo [INFO] Redis Connection: !REDIS_HOST!:!REDIS_PORT!

set "RABBIT_HOST="
set /p RABBIT_HOST=Enter RabbitMQ Host [127.0.0.1]: 
if "%RABBIT_HOST%"=="" set "RABBIT_HOST=127.0.0.1"
set "RABBIT_PORT="
set /p RABBIT_PORT=Enter RabbitMQ Port [5672]: 
if "%RABBIT_PORT%"=="" set "RABBIT_PORT=5672"
set "RABBIT_UID="
set /p RABBIT_UID=Enter RabbitMQ User ID [guest]: 
if "%RABBIT_UID%"=="" set "RABBIT_UID=guest"
set "RABBIT_PWD="
set /p RABBIT_PWD=Enter RabbitMQ Password [guest]: 
if "%RABBIT_PWD%"=="" set "RABBIT_PWD=guest"
echo [INFO] RabbitMQ Connection: !RABBIT_HOST!:!RABBIT_PORT! user=!RABBIT_UID!

:: ======================================================
:: 4) Verify MySQL availability via TCP port (using netstat)
::    (If not listening, go to error)
:: ======================================================
netstat -an | find ":!MYSQL_PORT! " | find "LISTENING" >nul
if errorlevel 1 (
    echo [ERROR] MySQL not listening on port !MYSQL_PORT! locally.
    goto :ErrorExit
)
echo [INFO] MySQL is listening on port !MYSQL_PORT!.
echo [INFO] Applying latest.sql...
mysql --protocol=tcp --host=!MYSQL_SERVER! --user=!MYSQL_USER! --password=!MYSQL_PWD! --port=!MYSQL_PORT! --default-character-set=utf8 --comments < infra\db\latest.sql
if errorlevel 1 (
    echo [ERROR] Failed to apply latest.sql.
    goto :ErrorExit
)
echo [INFO] latest.sql applied successfully.

:: ======================================================
:: 5) Verify Redis and RabbitMQ via TCP port (using netstat)
::    (If either not listening, go to error)
:: ======================================================
netstat -an | find ":!REDIS_PORT! " | find "LISTENING" >nul
if errorlevel 1 (
    echo [ERROR] Redis not listening on port !REDIS_PORT! locally.
    goto :ErrorExit
)
echo [INFO] Redis is listening on port !REDIS_PORT!.

netstat -an | find ":!RABBIT_PORT! " | find "LISTENING" >nul
if errorlevel 1 (
    echo [ERROR] RabbitMQ not listening on port !RABBIT_PORT! locally.
    goto :ErrorExit
)
echo [INFO] RabbitMQ is listening on port !RABBIT_PORT!.

:: ======================================================
:: 6) Generate game/config/config.dev.json
:: ======================================================
set "GAME_CONFIG_DIR=game\config"
set "GAME_CONFIG_PATH=%GAME_CONFIG_DIR%\config.dev.json"
if not exist "%GAME_CONFIG_DIR%" mkdir "%GAME_CONFIG_DIR%"
if exist "%GAME_CONFIG_PATH%" del /f /q "%GAME_CONFIG_PATH%"
(
    echo {
    echo     "id": 0,
    echo     "name": "game-0",
    echo     "delay": 5,
    echo     "ip": "!IP!",
    echo     "port": 3004,
    echo     "thread": {
    echo         "logic": 12,
    echo         "io": 12
    echo     },
    echo     "save": 600,
    echo     "internal": {
    echo         "ip": "127.0.0.1",
    echo         "port": 3000
    echo     },
    echo     "redis": {
    echo         "ip": "!REDIS_HOST!",
    echo         "port": !REDIS_PORT!,
    echo         "pool": 2
    echo     },
    echo     "login": {
    echo         "ip": "!IP!",
    echo         "port": 3002
    echo     },
    echo     "amqp": {
    echo         "ip": "!RABBIT_HOST!",
    echo         "port": !RABBIT_PORT!,
    echo         "uid": "!RABBIT_UID!",
    echo         "pwd": "!RABBIT_PWD!"
    echo     },
    echo     "log": [
    echo         "debug",
    echo         "info",
    echo         "warn",
    echo         "fatal"
    echo     ]
    echo }
) > "%GAME_CONFIG_PATH%"
echo [INFO] Created "%GAME_CONFIG_PATH%"

:: ======================================================
:: 7) Generate login/config/config.dev.json
:: ======================================================
set "LOGIN_CONFIG_DIR=login\config"
set "LOGIN_CONFIG_PATH=%LOGIN_CONFIG_DIR%\config.dev.json"
if not exist "%LOGIN_CONFIG_DIR%" mkdir "%LOGIN_CONFIG_DIR%"
if exist "%LOGIN_CONFIG_PATH%" del /f /q "%LOGIN_CONFIG_PATH%"
(
    echo {
    echo     "id": 0,
    echo     "name": "login-0",
    echo     "ip": "!IP!",
    echo     "port": 3002,
    echo     "transfer delay": 0,
    echo     "allow_foreign_name": false,
    echo     "agreement": "안녕하세요",
    echo     "admin_mode": false,
    echo     "thread": {
    echo         "logic": 12,
    echo         "io": 12
    echo     },
    echo     "amqp": {
    echo         "ip": "!RABBIT_HOST!",
    echo         "port": !RABBIT_PORT!,
    echo         "uid": "!RABBIT_UID!",
    echo         "pwd": "!RABBIT_PWD!"
    echo     },
    echo     "redis": {
    echo         "ip": "!REDIS_HOST!",
    echo         "port": !REDIS_PORT!,
    echo         "pool": 2
    echo     },
    echo     "internal": {
    echo         "ip": "127.0.0.1",
    echo         "port": 3000
    echo     },
    echo     "log": [
    echo         "debug",
    echo         "info",
    echo         "warn",
    echo         "fatal"
    echo     ],
    echo     "init": {
    echo         "map": 1,
    echo         "position": [
    echo             {
    echo                 "x": 6,
    echo                 "y": 6
    echo             }
    echo         ],
    echo         "hp": {
    echo             "base": 50,
    echo             "range": 10
    echo         },
    echo         "mp": {
    echo             "base": 50,
    echo             "range": 10
    echo         }
    echo     },
    echo     "name_size": {
    echo         "min": 2,
    echo         "max": 256
    echo     },
    echo     "pw_size": {
    echo         "min": 4,
    echo         "max": 14
    echo     }
    echo }
) > "%LOGIN_CONFIG_PATH%"
echo [INFO] Created "%LOGIN_CONFIG_PATH%"

:: ======================================================
:: 8) Generate gateway/config/config.dev.json
:: ======================================================
set "GATEWAY_CONFIG_DIR=gateway\config"
set "GATEWAY_CONFIG_PATH=%GATEWAY_CONFIG_DIR%\config.dev.json"
if not exist "%GATEWAY_CONFIG_DIR%" mkdir "%GATEWAY_CONFIG_DIR%"
if exist "%GATEWAY_CONFIG_PATH%" del /f /q "%GATEWAY_CONFIG_PATH%"
(
    echo {
    echo     "id": 0,
    echo     "ip": "!IP!",
    echo     "name": "gateway",
    echo     "port": 3001,
    echo     "thread": {
    echo         "logic": 12,
    echo         "io": 12
    echo     },
    echo     "log": [
    echo         "debug",
    echo         "info",
    echo         "warn",
    echo         "fatal"
    echo     ],
    echo     "entrypoints": [
    echo         {
    echo             "name": "로그인서버",
    echo             "desc": "설명을 입력하세요",
    echo             "ip": "!IP!",
    echo             "port": 3002
    echo         }
    echo     ],
    echo     "amqp": {
    echo         "ip": "!RABBIT_HOST!",
    echo         "port": !RABBIT_PORT!,
    echo         "uid": "!RABBIT_UID!",
    echo         "pwd": "!RABBIT_PWD!"
    echo     }
    echo }
) > "%GATEWAY_CONFIG_PATH%"
echo [INFO] Created "%GATEWAY_CONFIG_PATH%"

:: ======================================================
:: 9) Generate internal/appsettings.Development.json
:: ======================================================
set "INTERNAL_DIR=internal"
set "APPSETTINGS_PATH=%INTERNAL_DIR%\appsettings.Development.json"
if not exist "%INTERNAL_DIR%" mkdir "%INTERNAL_DIR%"
if exist "%APPSETTINGS_PATH%" del /f /q "%APPSETTINGS_PATH%"
(
    echo {
    echo     "Logging": {
    echo         "LogLevel": {
    echo             "Default": "Information",
    echo             "Microsoft.AspNetCore": "Warning"
    echo         }
    echo     },
    echo     "ConnectionStrings": {
    echo         "MySql": {
    echo             "-1": "Server=!MYSQL_SERVER!;Port=!MYSQL_PORT!;User ID=!MYSQL_USER!;Password=!MYSQL_PWD!;Database=!MYSQL_DB!",
    echo             "0":  "Server=!MYSQL_SERVER!;Port=!MYSQL_PORT!;User ID=!MYSQL_USER!;Password=!MYSQL_PWD!;Database=!MYSQL_DB!"
    echo         }
    echo     },
    echo     "Redis": {
    echo         "-1": {
    echo             "Host": "!REDIS_HOST!",
    echo             "Port": !REDIS_PORT!
    echo         },
    echo         "0": {
    echo             "Host": "!REDIS_HOST!",
    echo             "Port": !REDIS_PORT!
    echo         }
    echo     },
    echo     "RabbitMQ": {
    echo         "Host": "!RABBIT_HOST!",
    echo         "Port": !RABBIT_PORT!,
    echo         "Uid": "!RABBIT_UID!",
    echo         "Pwd": "!RABBIT_PWD!"
    echo     },
    echo     "Urls": "http://localhost:3000"
    echo }
) > "%APPSETTINGS_PATH%"
echo [INFO] Created "%APPSETTINGS_PATH%"

:: ======================================================
:: 10) Generate write-back/appsettings.json
:: ======================================================
set "WRITEBACK_DIR=write-back"
set "WRITEBACK_PATH=%WRITEBACK_DIR%\appsettings.json"
if not exist "%WRITEBACK_DIR%" mkdir "%WRITEBACK_DIR%"
if exist "%WRITEBACK_PATH%" del /f /q "%WRITEBACK_PATH%"
(
    echo {
    echo     "Logging": {
    echo         "LogLevel": {
    echo             "Default": "Information",
    echo             "Microsoft.AspNetCore": "Warning"
    echo         }
    echo     },
    echo     "ConnectionStrings": {
    echo         "MySql": {
    echo             "-1": "Server=!MYSQL_SERVER!;Port=!MYSQL_PORT!;User ID=!MYSQL_USER!;Password=!MYSQL_PWD!;Database=!MYSQL_DB!",
    echo             "0":  "Server=!MYSQL_SERVER!;Port=!MYSQL_PORT!;User ID=!MYSQL_USER!;Password=!MYSQL_PWD!;Database=!MYSQL_DB!"
    echo         }
    echo     },
    echo     "Redis": {
    echo         "-1": {
    echo             "Host": "!REDIS_HOST!",
    echo             "Port": !REDIS_PORT!
    echo         },
    echo         "0": {
    echo             "Host": "!REDIS_HOST!",
    echo             "Port": !REDIS_PORT!
    echo         }
    echo     },
    echo     "RabbitMQ": {
    echo         "Host": "!RABBIT_HOST!",
    echo         "Port": !RABBIT_PORT!,
    echo         "Uid": "!RABBIT_UID!",
    echo         "Pwd": "!RABBIT_PWD!"
    echo     }
    echo }
) > "%WRITEBACK_PATH%"
echo [INFO] Created "%WRITEBACK_PATH%"

:: ======================================================
:: 11) Extract resources/maps/maps.zip into game/maps
:: ====================================================== 
set "MAPS_ZIP=resources\maps\maps.zip"
set "MAPS_DEST=game\maps"
if not exist "%MAPS_ZIP%" (
    echo [ERROR] "%MAPS_ZIP%" not found.
    goto :ErrorExit
)
if not exist "%MAPS_DEST%" mkdir "%MAPS_DEST%"
tar -xf "%MAPS_ZIP%" -C "%MAPS_DEST%"
if errorlevel 1 (
    echo [ERROR] Failed to extract "%MAPS_ZIP%" to "%MAPS_DEST%".
    goto :ErrorExit
)
echo [INFO] Extracted "%MAPS_ZIP%" to "%MAPS_DEST%"

:: ======================================================
:: 12) Run tools/update-*.bat if present
:: ======================================================
if exist "tools" (
    pushd "tools"
    if exist "update-data.bat" (
        echo [INFO] Running update-data.bat...
        call "update-data.bat"
        if errorlevel 1 (
            echo [ERROR] update-data.bat failed.
            popd
            goto :ErrorExit
        )
        echo [INFO] update-data.bat complete.
    ) else (
        echo [WARN] update-data.bat not found, skipping.
    )
    if exist "update-modules.bat" (
        echo [INFO] Running update-modules.bat...
        call "update-modules.bat"
        if errorlevel 1 (
            echo [ERROR] update-modules.bat failed.
            popd
            goto :ErrorExit
        )
        echo [INFO] update-modules.bat complete.
    ) else (
        echo [WARN] update-modules.bat not found, skipping.
    )
    if exist "update-flatbuffer.bat" (
        echo [INFO] Running update-flatbuffer.bat...
        call "update-flatbuffer.bat"
        if errorlevel 1 (
            echo [ERROR] update-flatbuffer.bat failed.
            popd
            goto :ErrorExit
        )
        echo [INFO] update-flatbuffer.bat complete.
    ) else (
        echo [WARN] update-flatbuffer.bat not found, skipping.
    )
    popd
) else (
    echo [WARN] "tools" directory not found, skipping tool updates.
)

:: ======================================================
:: 13) Create build directory and run CMake to generate solution
:: ======================================================
echo [INFO] Creating build directory and running CMake...
if not exist "build" mkdir "build"
cd build
cmake ..
cmake --build . --config Release
popd

goto :Success

:ErrorExit
    pause
    goto :EOF

:Success
    pause
    goto :EOF
