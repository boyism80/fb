@echo off

git submodule update --recursive --remote
PUSHD flatbuffer-ex
CALL dotnet publish -c Release -o "bin"
PUSHD bin
CALL FlatBufferEx.exe --path=..\..\protocol --lang="c++|c#" --include="fb/protocol/flatbuffer/raw"
POPD
POPD

if %ERRORLEVEL% NEQ 0 GOTO END

del /S /Q http\FlatBuffer
del /S /Q http\fb\protocol\flatbuffer
xcopy flatbuffer-ex\bin\output\c#\* http\FlatBuffer\* /S /E
robocopy flatbuffer-ex\bin\output\raw\c# http\fb\protocol\raw /S /E

del /S /Q include\fb\protocol\flatbuffer
xcopy flatbuffer-ex\bin\output\c++\* include\fb\protocol\flatbuffer\* /S /E
xcopy flatbuffer-ex\bin\output\raw\c++\* include\fb\protocol\flatbuffer\raw\* /S /E

GOTO SKIP_PAUSE
:END
PAUSE
:SKIP_PAUSE