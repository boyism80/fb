@echo off

git submodule update --recursive --remote
PUSHD flatbuffer-ex
CALL dotnet publish -c Release -o "bin"
PUSHD bin
CALL FlatBufferEx.exe --path=..\..\protocol --lang="c++|c#"
POPD
POPD

mkdir external-tool
pushd external-tool
curl -L -o flatbuffer.zip https://github.com/google/flatbuffers/releases/download/v24.3.25/Windows.flatc.binary.zip
tar -xf flatbuffer.zip
del flatbuffer.zip

flatc --cpp -o c++ ../protocol/monster.fbs ../protocol/response.fbs
flatc --csharp -o c# ../protocol/monster.fbs ../protocol/response.fbs
popd

pause