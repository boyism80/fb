@ECHO OFF
SET FNAME=latest.sql

mysql.exe --protocol=tcp --host=localhost --user=fb --password=admin --port=30100 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=localhost --user=fb --password=admin --port=30101 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=localhost --user=fb --password=admin --port=30102 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=localhost --user=fb --password=admin --port=30103 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=localhost --user=fb --password=admin --port=30104 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=localhost --user=fb --password=admin --port=30105 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=localhost --user=fb --password=admin --port=30106 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=localhost --user=fb --password=admin --port=30107 --default-character-set=utf8 --comments  < %FNAME%
ECHO DUMP COMPLETE : %FNAME%
