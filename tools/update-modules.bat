mkdir library
PUSHD library

SET DEST=..\..\dependency
SET BOOST=boost_1_84_0

git clone https://github.com/redis/hiredis.git
PUSHD hiredis
git checkout v1.2.0
if not exist build mkdir build
PUSHD build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -DENABLE_SSL_SUPPORT=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build . --config Debug --parallel
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DENABLE_SSL_SUPPORT=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build . --config Release --parallel
POPD
POPD
XCOPY hiredis\build\Debug\hiredisd.lib %DEST%\lib\hiredisd.* /K /D /H /Y
XCOPY hiredis\build\Release\hiredis.lib %DEST%\lib\hiredis.* /K /D /H /Y
ROBOCOPY hiredis\ %DEST%\include\hiredis\ *.h*


POPD