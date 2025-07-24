@echo off

git submodule update --recursive --remote flatbuffer-ex
PUSHD flatbuffer-ex
del /S /Q output
CALL dotnet publish -c Release -o "bin"
CALL bin\FlatBufferEx.exe --path=..\..\protocol --lang="c++|c#" --include="fb/protocol/flatbuffer/raw"
POPD

if %ERRORLEVEL% NEQ 0 GOTO END

PUSHD ..
del /S /Q server\http\FlatBuffer
del /S /Q server\http\fb\protocol\raw
xcopy tools\flatbuffer-ex\output\c#\* server\http\FlatBuffer\* /S /E
robocopy tools\flatbuffer-ex\output\raw\c# server\http\fb\protocol\raw /S /E

del /S /Q include\fb\protocol\flatbuffer
xcopy tools\flatbuffer-ex\output\c++\* include\fb\protocol\flatbuffer\* /S /E
xcopy tools\flatbuffer-ex\output\raw\c++\* include\fb\protocol\flatbuffer\raw\* /S /E
POPD

GOTO SKIP_PAUSE
:END
PAUSE
:SKIP_PAUSE