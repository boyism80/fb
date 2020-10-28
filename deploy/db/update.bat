@ECHO OFF
SET FNAME=latest.sql

mysql.exe --protocol=tcp --host=localhost --user=io --password=admin --port=3306 --default-character-set=utf8 --comments  < %FNAME%
ECHO DUMP COMPLETE : %FNAME%