@ECHO OFF
SET FNAME=latest.sql

ECHO MYSQL DATABASE DUMP TOOL MADE BY INOUT TEAM
mysqldump.exe --user=fb --password=admin --host=localhost --protocol=tcp --port=3306 --default-character-set=utf8 --no-data --skip-triggers --result-file=%FNAME% --add-drop-database --databases "fb"
ECHO.
ECHO.
ECHO.
ECHO DUMP COMPLETE : %FNAME%