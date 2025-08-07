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
CALL ExcelTableConverter.exe --dir=..\..\..\resources\table --lang="c++|c#|node|go" --dsl=..\..\..\resources\table\dsl.json --ns fb.model --additional-headers=model.additional.h
POPD
POPD

if ERRORLEVEL 1 GOTO END

PUSHD ..
DEL /s /q "include\fb\model\model.h"
DEL /s /q "include\fb\model\datetime.h"
RMDIR /s /q "server\game\json"
RMDIR /s /q "server\login\json"
RMDIR /s /q "server\bot\json"
COPY "tools\data-converter\bin\output\C++\server\model.h" "include\fb\model\model.h"
COPY "tools\data-converter\bin\output\C++\datetime.h" "include\fb\model\datetime.h"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "server\game\json\*.json"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "server\login\json\*.json"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "server\bot\json\*.json"

DEL /s /q "server\http\Model\Model.cs"
RMDIR /s /q "server\internal\json"
XCOPY "tools\data-converter\bin\output\C#\server\*.cs" "server\http\Model\*.cs"
XCOPY "tools\data-converter\bin\output\json\server\*.json" "server\internal\json\*.json"
POPD

GOTO SKIP_PAUSE
:END
PAUSE
:SKIP_PAUSE