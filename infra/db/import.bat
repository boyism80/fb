@ECHO OFF
SET FNAME=latest.sql

REM Unified infrastructure MySQL (global)
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=31000 --default-character-set=utf8 --comments  < %FNAME%

REM World-1 (연) MySQL
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30100 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30101 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30102 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30103 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30104 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30105 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30106 --default-character-set=utf8 --comments  < %FNAME%

REM World-2 (무휼) MySQL
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30120 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30121 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30122 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30123 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30124 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30125 --default-character-set=utf8 --comments  < %FNAME%
mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30126 --default-character-set=utf8 --comments  < %FNAME%

REM World-3 (세류) MySQL
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30140 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30141 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30142 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30143 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30144 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30145 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30146 --default-character-set=utf8 --comments  < %FNAME%

REM World-4 (유리) MySQL
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30160 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30161 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30162 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30163 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30164 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30165 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30166 --default-character-set=utf8 --comments  < %FNAME%

REM World-5 (해명) MySQL
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30180 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30181 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30182 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30183 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30184 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30185 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30186 --default-character-set=utf8 --comments  < %FNAME%

REM World-6 (배극) MySQL
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30200 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30201 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30202 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30203 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30204 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30205 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30206 --default-character-set=utf8 --comments  < %FNAME%

REM World-7 (하자) MySQL
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30220 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30221 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30222 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30223 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30224 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30225 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30226 --default-character-set=utf8 --comments  < %FNAME%

REM World-8 (호동) MySQL
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30240 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30241 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30242 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30243 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30244 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30245 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30246 --default-character-set=utf8 --comments  < %FNAME%

REM World-9 (주몽) MySQL
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30260 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30261 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30262 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30263 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30264 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30265 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30266 --default-character-set=utf8 --comments  < %FNAME%

REM World-10 (낙랑) MySQL
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30280 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30281 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30282 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30283 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30284 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30285 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30286 --default-character-set=utf8 --comments  < %FNAME%

REM World-11 (괴유) MySQL
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30300 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30301 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30302 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30303 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30304 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30305 --default-character-set=utf8 --comments  < %FNAME%
@REM mysql.exe --protocol=tcp --host=192.168.0.180 --user=fb --password=admin --port=30306 --default-character-set=utf8 --comments  < %FNAME%

ECHO IMPORT COMPLETE : %FNAME%
