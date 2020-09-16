SET DISTRIBUTION=1_74_0
SET DIRECTORY=boost_%DISTRIBUTION%
SET FILE_NAME=%DIRECTORY%.zip
SET BOOST_URL=https://dl.bintray.com/boostorg/release/1.74.0/source/%FILE_NAME%
SET DRIVE=D:\

IF NOT EXIST %FILE_NAME% curl https://dl.bintray.com/boostorg/release/1.74.0/source/%DIRECTORY%.zip -o %FILE_NAME% -L

IF NOT EXIST %DRIVE% SET DRIVE=C:\
SET ABSOLUTE_PATH=%DRIVE%%DIRECTORY%

tar -xzf %FILE_NAME% -C %DRIVE%


PUSHD %ABSOLUTE_PATH%
CALL bootstrap.bat
b2 -j4 -a --toolset=msvc-14.2 variant=debug,release link=static runtime-link=static threading=multi address-model=32 stage
b2 -j4 -a --toolset=msvc-14.2 variant=debug,release link=static runtime-link=static threading=multi address-model=64 stage
POPD

DEL %FILE_NAME%
SETX BOOST_SDK "%ABSOLUTE_PATH%"
PAUSE