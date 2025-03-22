@ECHO OFF
SET FNAME=latest.sql

ECHO MYSQL DATABASE DUMP TOOL MADE BY INOUT TEAM
mysqldump.exe --user=fb --password=admin --host=127.0.0.1 --protocol=tcp --port=3306 --default-character-set=utf8 --no-data --skip-triggers --routines --result-file=%FNAME% --add-drop-database --databases "fb"
ECHO DUMP COMPLETE : %FNAME%
PAUSE