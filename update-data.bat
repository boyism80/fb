@ECHO OFF

git submodule update --recursive --remote

SET SOURCE=D:\Users\CSHYEON\Data\git\game\c++\fb
PUSHD data-converter
CALL dotnet publish -c Release -o "bin"
if ERRORLEVEL 1 GOTO END
PUSHD bin
CALL ExcelTableConverter.exe --dir=..\..\resources\table --lang="c++|c#"
POPD
POPD

if ERRORLEVEL 1 GOTO END

DEL /s /q "include\fb\game\model.h"
RMDIR /s /q "game\json"
XCOPY "data-converter\bin\output\C++\server\*.h" "include\fb\game\*.h"
XCOPY "data-converter\bin\output\json\server\*.json" "game\json\*.json"

DEL /s /q "internal\Model\Model.cs"
RMDIR /s /q "internal\json"
XCOPY "data-converter\bin\output\C#\server\*.cs" "internal\Model\*.cs"
XCOPY "data-converter\bin\output\json\server\*.json" "internal\json\*.json"

GOTO SKIP_PAUSE
:END
PAUSE
:SKIP_PAUSE