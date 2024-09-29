@echo off

git submodule update --recursive --remote
PUSHD flatbuffer-ex
CALL dotnet publish -c Release -o "bin"
PUSHD bin
CALL FlatBufferEx.exe --path=..\..\protocol --lang="c++|c#" --include="fb/protocol"
POPD
POPD

del /S /Q http\fb\protocol\flatbuffer
robocopy flatbuffer-ex\bin\output\c# http /S /E
robocopy flatbuffer-ex\bin\flatbuffer\c# http\fb\protocol\origin /S /E
move http\IFlatBufferEx.cs http\fb\protocol\IFlatBufferEx.cs

del /S /Q include\fb\protocol\flatbuffer
robocopy flatbuffer-ex\bin\output\c++ include /S /E
robocopy flatbuffer-ex\bin\flatbuffer\c++ include\fb\protocol\origin /S /E
move include\protocol_type.h include\fb\protocol\protocol_type.h

pause