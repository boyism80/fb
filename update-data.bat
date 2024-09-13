@ECHO OFF

git submodule update --recursive --remote

SET SOURCE=D:\Users\CSHYEON\Data\git\game\c++\fb
PUSHD data-converter
CALL dotnet publish -c Release -o "bin"
if ERRORLEVEL 1 GOTO END
PUSHD bin
CALL ExcelTableConverter.exe --dir=..\..\resources\table --lang=c++
POPD
POPD

if ERRORLEVEL 1 GOTO END

DEL /s /q "include\fb\game\model.h"
RMDIR /s /q "game\json"
XCOPY "data-converter\bin\output\C++\server\*.h" "%SOURCE%\include\fb\game\*.h"
XCOPY "data-converter\bin\output\json\\server\*.json" "%SOURCE%\game\json\*.json"

GOTO SKIP_PAUSE
:END
PAUSE
:SKIP_PAUSE