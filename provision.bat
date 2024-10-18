REM MYSQL C CONNECTOR
SET DISTRIBUTION=6.1.11
SET DIRECTORY=mysql-connector-c-%DISTRIBUTION%-winx64
SET FILE_NAME=%DIRECTORY%.zip
SET MYSQL_CONNECTOR_URL=https://cdn.mysql.com/archives/mysql-connector-c/mysql-connector-c-%DISTRIBUTION%-winx64.zip
SET DRIVE=D:\

IF NOT EXIST %FILE_NAME% curl %MYSQL_CONNECTOR_URL% -o %FILE_NAME% -L

IF NOT EXIST %DRIVE% SET DRIVE=C:\
SET ABSOLUTE_PATH=%DRIVE%%DIRECTORY%

tar -xzf %FILE_NAME%
MOVE %DIRECTORY% %DRIVE%

DEL %FILE_NAME%
SETX MYSQL_SDK "%ABSOLUTE_PATH%"


REM BOOST
SET DISTRIBUTION=1_84_0
SET DIRECTORY=boost_%DISTRIBUTION%
SET FILE_NAME=%DIRECTORY%.zip
SET BOOST_URL=https://dl.bintray.com/boostorg/release/1.84.0/source/%FILE_NAME%
SET DRIVE=D:\

IF NOT EXIST %FILE_NAME% curl %BOOST_URL% -o %FILE_NAME% -L

IF NOT EXIST %DRIVE% SET DRIVE=C:\
SET ABSOLUTE_PATH=%DRIVE%%DIRECTORY%

tar -xzf %FILE_NAME% -C %DRIVE%


PUSHD %ABSOLUTE_PATH%
CALL bootstrap.bat
b2 -j4 -a --toolset=msvc-14.3 variant=debug,release link=static runtime-link=static threading=multi address-model=32 stage
b2 -j4 -a --toolset=msvc-14.3 variant=debug,release link=static runtime-link=static threading=multi address-model=64 stage
POPD

DEL %FILE_NAME%
SETX BOOST_SDK "%ABSOLUTE_PATH%"
PAUSE