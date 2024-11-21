@ECHO OFF

git submodule update --recursive --remote data-converter
SET SOURCE=D:\Users\CSHYEON\Data\git\game\c++\fb
PUSHD data-converter
CALL dotnet publish -c Release -o "bin"
if ERRORLEVEL 1 GOTO END
PUSHD bin
CALL ExcelTableConverter.exe --dir=..\..\..\resources\table --lang="c++|c#"
POPD
POPD

if ERRORLEVEL 1 GOTO END

PUSHD ..
DEL /s /q "include\fb\model\model.h"
DEL /s /q "include\fb\model\datetime.h"
RMDIR /s /q "game\json"
RMDIR /s /q "login\json"
COPY "tools\data-converter\bin\output\C++\server\model.h" "include\fb\model\model.h"
COPY "tools\data-converter\bin\output\C++\datetime.h" "include\fb\model\datetime.h"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "game\json\*.json"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "login\json\*.json"

DEL /s /q "http\Model\Model.cs"
RMDIR /s /q "internal\json"
XCOPY "tools\data-converter\bin\output\C#\server\*.cs" "http\Model\*.cs"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "internal\json\*.json"
POPD

GOTO SKIP_PAUSE
:END
PAUSE
:SKIP_PAUSE