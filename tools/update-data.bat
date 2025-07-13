@ECHO OFF

git submodule update --recursive --remote data-converter
PUSHD data-converter

SET DISABLE_TTY=%1

IF "%1" == "true" (
	dotnet publish -c Release /p:DefineConstants=DISABLED_TTY -o bin
) ELSE (
	dotnet publish -c Release -o bin
)

if ERRORLEVEL 1 GOTO END
PUSHD bin
CALL ExcelTableConverter.exe --dir=..\..\..\resources\table --lang="c++|c#|node|go" --dsl=..\..\..\resources\table\dsl.json
POPD
POPD

if ERRORLEVEL 1 GOTO END

PUSHD ..
DEL /s /q "include\fb\model\model.h"
DEL /s /q "include\fb\model\datetime.h"
RMDIR /s /q "game\json"
RMDIR /s /q "login\json"
RMDIR /s /q "bot\json"
COPY "tools\data-converter\bin\output\C++\server\model.h" "include\fb\model\model.h"
COPY "tools\data-converter\bin\output\C++\datetime.h" "include\fb\model\datetime.h"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "game\json\*.json"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "login\json\*.json"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "bot\json\*.json"

DEL /s /q "http\Model\Model.cs"
RMDIR /s /q "internal\json"
XCOPY "tools\data-converter\bin\output\C#\server\*.cs" "http\Model\*.cs"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "internal\json\*.json"
POPD

GOTO SKIP_PAUSE
:END
PAUSE
:SKIP_PAUSE