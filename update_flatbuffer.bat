@echo off

git submodule update --recursive --remote
PUSHD flatbuffer-ex
CALL dotnet publish -c Release -o "bin"
PUSHD bin
CALL FlatBufferEx.exe --path=..\..\protocol --lang="c++|c#" --include="fb/protocol/flatbuffer"
POPD
POPD

del /S /Q http\fb\protocol\flatbuffer
robocopy flatbuffer-ex\bin\output\c# http\fb\protocol\flatbuffer /S /E
robocopy flatbuffer-ex\bin\flatbuffer\c# http\fb\protocol\flatbuffer\origin /S /E

del /S /Q include\fb\protocol\flatbuffer
robocopy flatbuffer-ex\bin\output\c++ include\fb\protocol\flatbuffer /S /E
robocopy flatbuffer-ex\bin\flatbuffer\c++ include\fb\protocol\flatbuffer\origin /S /E

pause