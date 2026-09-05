@echo off
setlocal

REM Pinned refs — keep server/fb/Dockerfile ARG defaults in sync when bumping below.
set JSONCPP_REF=1.9.6
set RABBITMQ_REF=v0.14.0
set ZLIB_REF=v1.2.9
set CPP_ASYNC_REF=v1.1.3
set BOOST_REF=boost-1.84.0

mkdir library 2>nul
PUSHD library

SET DEST=..\..\dependency

call :ensure_git_repo https://github.com/open-source-parsers/jsoncpp jsoncpp %JSONCPP_REF%
PUSHD jsoncpp
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DJSONCPP_STATIC_WINDOWS_RUNTIME=ON -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --target jsoncpp_static --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DJSONCPP_STATIC_WINDOWS_RUNTIME=ON -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --target jsoncpp_static --config Release --parallel
POPD
POPD
XCOPY jsoncpp\build\lib\Debug\jsoncpp_static.lib %DEST%\lib\jsoncppd.* /K /D /H /Y
XCOPY jsoncpp\build\lib\Release\jsoncpp_static.lib %DEST%\lib\jsoncpp.* /K /D /H /Y
ROBOCOPY jsoncpp\include\json\ %DEST%\include\json\

call :ensure_git_repo https://github.com/alanxz/rabbitmq-c rabbitmq-c %RABBITMQ_REF%
PUSHD rabbitmq-c
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -DENABLE_SSL_SUPPORT=OFF
cmake --build . --target rabbitmq-static --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DENABLE_SSL_SUPPORT=OFF
cmake --build . --target rabbitmq-static --config Release --parallel
POPD
POPD
XCOPY rabbitmq-c\build\librabbitmq\Debug\librabbitmq.4.lib %DEST%\lib\librabbitmq.4d.* /K /D /H /Y
XCOPY rabbitmq-c\build\librabbitmq\Release\librabbitmq.4.lib %DEST%\lib\librabbitmq.4.* /K /D /H /Y
ROBOCOPY rabbitmq-c\include\rabbitmq-c\ %DEST%\include\rabbitmq-c\ *.h*
ROBOCOPY rabbitmq-c\build\include\rabbitmq-c\ %DEST%\include\rabbitmq-c\ *.h*

call :ensure_git_repo https://github.com/boyism80/lua lua "" --recursive
PUSHD lua
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --config Release --parallel
POPD
POPD
XCOPY lua\build\Debug\lua.lib %DEST%\lib\luad.* /K /D /H /Y
XCOPY lua\build\Release\lua.lib %DEST%\lib\lua.* /K /D /H /Y
ROBOCOPY lua\upstream\ %DEST%\include\lua\ *.h*

call :ensure_git_repo https://github.com/intel/zlib zlib %ZLIB_REF%
PUSHD zlib
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --config Release --parallel
POPD
POPD
XCOPY zlib\build\Debug\zlibstaticd.lib %DEST%\lib\zlibd.* /K /D /H /Y
XCOPY zlib\build\Release\zlibstatic.lib %DEST%\lib\zlib.* /K /D /H /Y
XCOPY zlib\build\zconf.h %DEST%\include\zlib\zconf.* /K /D /H /Y
ROBOCOPY zlib\ %DEST%\include\zlib\ *.h

call :ensure_git_repo https://github.com/boyism80/flatbuffers flatbuffers
PUSHD flatbuffers
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -DFLATBUFFERS_BUILD_FLATC=OFF -DFLATBUFFERS_BUILD_TESTS=OFF -DFLATBUFFERS_INSTALL=OFF
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DFLATBUFFERS_BUILD_FLATC=OFF -DFLATBUFFERS_BUILD_TESTS=OFF -DFLATBUFFERS_INSTALL=OFF
cmake --build . --config Release --parallel
POPD
POPD
XCOPY flatbuffers\build\Debug\flatbuffers.lib %DEST%\lib\flatbuffersd.* /K /D /H /Y
XCOPY flatbuffers\build\Release\flatbuffers.lib %DEST%\lib\flatbuffers.* /K /D /H /Y
ROBOCOPY flatbuffers\include\ %DEST%\include\ /E

call :ensure_git_repo https://github.com/boyism80/cpp-async cpp-async %CPP_ASYNC_REF%
ROBOCOPY cpp-async\include\async\ %DEST%\include\async\ *.h /MIR /NFL /NDL /NJH /NJS /NP
if %ERRORLEVEL% GEQ 8 exit /b %ERRORLEVEL%

REM No pinned ref; tracks default branch.
call :ensure_git_repo https://github.com/cjgdev/aho_corasick aho_corasick
ROBOCOPY aho_corasick\src\aho_corasick\ %DEST%\include\aho_corasick\ *.hpp /MIR /NFL /NDL /NJH /NJS /NP
if %ERRORLEVEL% GEQ 8 exit /b %ERRORLEVEL%

REM No pinned ref; tracks default branch. Header-only cron expression library.
call :ensure_git_repo https://github.com/mariusbancila/croncpp croncpp
if not exist "%DEST%\include" mkdir "%DEST%\include"
COPY /Y croncpp\include\croncpp.h %DEST%\include\croncpp.h >nul
if ERRORLEVEL 1 exit /b %ERRORLEVEL%

call :ensure_git_repo https://github.com/boostorg/boost boost %BOOST_REF%
PUSHD boost
git submodule update --init --recursive
CALL bootstrap.bat
CALL b2.exe --build-type=complete --layout=versioned runtime-link=static threading=multi variant=debug,release
POPD
ROBOCOPY boost\boost\ %DEST%\include\boost\ /E
ROBOCOPY boost\stage\lib\ %DEST%\lib\boost\ /E *.lib

POPD
exit /b 0

REM Clone when missing; fetch and checkout the pinned ref when the repo already exists.
REM Usage: call :ensure_git_repo <url> <dir> [ref] [clone-options]
:ensure_git_repo
if not exist "%~2" (
    if "%~4"=="" (
        git clone "%~1" "%~2"
    ) else (
        git clone %~4 "%~1" "%~2"
    )
    if not "%~3"=="" (
        pushd "%~2"
        git checkout "%~3"
        popd
    )
) else if not "%~3"=="" (
    pushd "%~2"
    git fetch origin --tags
    git checkout "%~3"
    popd
)
exit /b 0
