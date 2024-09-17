@echo off

git submodule update --recursive --remote
PUSHD flatbuffer-ex
CALL dotnet publish -c Release -o "bin"
PUSHD bin
CALL FlatBufferEx.exe --path=..\..\protocol --lang="c++|c#" --include="fb/protocol/flatbuffer"
POPD
POPD

mkdir external-tool
pushd external-tool
curl -L -o flatbuffer.zip https://github.com/google/flatbuffers/releases/download/v24.3.25/Windows.flatc.binary.zip
tar -xf flatbuffer.zip
del flatbuffer.zip

del /S /Q c++
del /S /Q c#
for %%f in (..\protocol\*.fbs) do (
	flatc --cpp -o c++ %%f
	flatc --csharp -o c# %%f
)
robocopy c# ..\http /S /E
robocopy c++ ..\include\fb\protocol\flatbuffer /S /E
popd

del /S /Q http\fb\protocol\flatbuffer\model
robocopy flatbuffer-ex\bin\output\c# http\fb\protocol\flatbuffer\model /S /E

del /S /Q include\fb\protocol\flatbuffer\model
robocopy flatbuffer-ex\bin\output\c++ include\fb\protocol\flatbuffer\model /S /E

pause