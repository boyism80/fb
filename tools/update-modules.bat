mkdir library
PUSHD library

SET DEST=..\..\dependency
SET BOOST=boost_1_84_0



git clone https://github.com/open-source-parsers/jsoncpp
PUSHD jsoncpp
git checkout 1.9.6
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DJSONCPP_STATIC_WINDOWS_RUNTIME=ON -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --target jsoncpp_static --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DJSONCPP_STATIC_WINDOWS_RUNTIME=ON -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --target jsoncpp_static --config Release --parallel
POPD
POPD
XCOPY jsoncpp\build\lib\Debug\jsoncpp_static.lib %DEST%\lib\jsoncppd.* /K /D /H /Y
XCOPY jsoncpp\build\lib\Release\jsoncpp_static.lib %DEST%\lib\jsoncpp.* /K /D /H /Y
ROBOCOPY jsoncpp\include\json\ %DEST%\include\json\

git clone https://github.com/alanxz/rabbitmq-c
PUSHD rabbitmq-c
git checkout v0.14.0
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -DENABLE_SSL_SUPPORT=OFF
cmake --build . --target rabbitmq-static --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DENABLE_SSL_SUPPORT=OFF
cmake --build . --target rabbitmq-static --config Release --parallel
POPD
POPD
XCOPY rabbitmq-c\build\librabbitmq\Debug\librabbitmq.4.lib %DEST%\lib\librabbitmq.4d.* /K /D /H /Y
XCOPY rabbitmq-c\build\librabbitmq\Release\librabbitmq.4.lib %DEST%\lib\librabbitmq.4.* /K /D /H /Y
ROBOCOPY rabbitmq-c\include\rabbitmq-c\ %DEST%\include\rabbitmq-c\ *.h*
ROBOCOPY rabbitmq-c\build\include\rabbitmq-c\ %DEST%\include\rabbitmq-c\ *.h*

git clone --recursive https://github.com/boyism80/lua
PUSHD lua
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --config Release --parallel
POPD
POPD
XCOPY lua\build\Debug\lua.lib %DEST%\lib\luad.* /K /D /H /Y
XCOPY lua\build\Release\lua.lib %DEST%\lib\lua.* /K /D /H /Y
ROBOCOPY lua\upstream\ %DEST%\include\lua\ *.h*

git clone https://github.com/intel/zlib
PUSHD zlib
git checkout v1.2.9
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --config Release --parallel
POPD
POPD
XCOPY zlib\build\Debug\zlibstaticd.lib %DEST%\lib\zlibd.* /K /D /H /Y
XCOPY zlib\build\Release\zlibstatic.lib %DEST%\lib\zlib.* /K /D /H /Y
XCOPY zlib\build\zconf.h %DEST%\include\zlib\zconf.* /K /D /H /Y
ROBOCOPY zlib\ %DEST%\include\zlib\ *.h

git clone https://github.com/boyism80/flatbuffers
PUSHD flatbuffers
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -DFLATBUFFERS_BUILD_FLATC=OFF -DFLATBUFFERS_BUILD_TESTS=OFF -DFLATBUFFERS_INSTALL=OFF
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DFLATBUFFERS_BUILD_FLATC=OFF -DFLATBUFFERS_BUILD_TESTS=OFF -DFLATBUFFERS_INSTALL=OFF
cmake --build . --config Release --parallel
POPD
POPD
XCOPY flatbuffers\build\Debug\flatbuffers.lib %DEST%\lib\flatbuffersd.* /K /D /H /Y
XCOPY flatbuffers\build\Release\flatbuffers.lib %DEST%\lib\flatbuffers.* /K /D /H /Y
ROBOCOPY flatbuffers\include\ %DEST%\include\ /E

git clone https://github.com/microsoft/cpp-async
PUSHD cpp-async
git checkout v1.1.0
POPD
ROBOCOPY cpp-async\include\async\ %DEST%\include\async\ *.h

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

git clone https://github.com/boostorg/boost
PUSHD boost
git checkout boost-1.84.0
git submodule update --init --recursive
CALL bootstrap.bat
CALL b2.exe --build-type=complete --layout=tagged runtime-link=static threading=multi variant=debug,release
POPD
ROBOCOPY boost\boost\ %DEST%\include\boost\ /E
ROBOCOPY boost\stage\lib\ %DEST%\lib\boost\ /E *.lib

POPD
exit /b 0