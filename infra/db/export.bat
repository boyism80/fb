@ECHO OFF
SET FNAME=latest.sql

ECHO MYSQL DATABASE DUMP TOOL MADE BY INOUT TEAM
mysqldump.exe --user=fb --password=admin --host=localhost --protocol=tcp --port=3306 --default-character-set=utf8 --no-data --skip-triggers --routines --result-file=%FNAME% --add-drop-database --databases "fb"


mysql.exe --protocol=tcp --host=localhost --user=fb --password=admin --port=30000 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=localhost --user=fb --password=admin --port=30001 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=localhost --user=fb --password=admin --port=30002 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=localhost --user=fb --password=admin --port=30003 --default-character-set=utf8 --comments  < %FNAME%
ECHO DUMP COMPLETE : %FNAME%