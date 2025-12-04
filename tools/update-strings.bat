@echo off
REM 엑셀 파일 생성 스크립트 실행 배치 파일

setlocal

REM 스크립트 디렉토리로 이동
cd /d "%~dp0generate-strings"

REM Python 스크립트 실행
python generate_const_string_excel.py

REM 오류 코드 확인
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo 오류: 스크립트 실행 중 오류가 발생했습니다.
    pause
    exit /b %ERRORLEVEL%
)

pause

