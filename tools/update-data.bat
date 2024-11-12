@ECHO OFF

rem git submodule update --recursive --remote data-converter
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
DEL /s /q "game\include\model.h"
DEL /s /q "include\fb\model\datetime.h"
RMDIR /s /q "game\json"
COPY "tools\data-converter\bin\output\C++\server\model.h" "game\include\model.h"
COPY "tools\data-converter\bin\output\C++\datetime.h" "include\fb\model\datetime.h"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "game\json\*.json"

DEL /s /q "http\Model\Model.cs"
RMDIR /s /q "internal\json"
RMDIR /s /q "db\json"
XCOPY "tools\data-converter\bin\output\C#\server\*.cs" "http\Model\*.cs"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "internal\json\*.json"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "db\json\*.json"
POPD

GOTO SKIP_PAUSE
:END
PAUSE
:SKIP_PAUSE