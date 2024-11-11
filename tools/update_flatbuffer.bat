@echo off

git submodule update --recursive --remote flatbuffer-ex
PUSHD flatbuffer-ex
CALL dotnet publish -c Release -o "bin"
CALL bin\FlatBufferEx.exe --path=..\..\protocol --lang="c++|c#" --include="fb/protocol/flatbuffer/raw"
POPD

if %ERRORLEVEL% NEQ 0 GOTO END

PUSHD ..
del /S /Q http\FlatBuffer
del /S /Q http\fb\protocol\flatbuffer
xcopy tools\flatbuffer-ex\output\c#\* http\FlatBuffer\* /S /E
robocopy tools\flatbuffer-ex\output\raw\c# http\fb\protocol\raw /S /E

del /S /Q include\fb\protocol\flatbuffer
xcopy tools\flatbuffer-ex\output\c++\* include\fb\protocol\flatbuffer\* /S /E
xcopy tools\flatbuffer-ex\output\raw\c++\* include\fb\protocol\flatbuffer\raw\* /S /E
POPD

GOTO SKIP_PAUSE
:END
PAUSE
:SKIP_PAUSE