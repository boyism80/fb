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

DEL /s /q "game\include\model.h"
DEL /s /q "include\fb\model\datetime.h"
RMDIR /s /q "game\json"
XCOPY "data-converter\bin\output\C++\server\*.h" "game\include\*.h"
XCOPY "data-converter\bin\output\C++\*.h" "include\fb\model\*.h"
XCOPY "data-converter\bin\output\json\server\*.json" "game\json\*.json"

DEL /s /q "http\Model\Model.cs"
RMDIR /s /q "internal\json"
RMDIR /s /q "db\json"
XCOPY "data-converter\bin\output\C#\server\*.cs" "http\Model\*.cs"
XCOPY "data-converter\bin\output\json\server\*.json" "internal\json\*.json"
XCOPY "data-converter\bin\output\json\server\*.json" "db\json\*.json"

GOTO SKIP_PAUSE
:END
PAUSE
:SKIP_PAUSE