mkdir modules
PUSHD modules

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
XCOPY jsoncpp\build\lib\Release\jsoncpp_static.lib %DEST%\lib\ /K /D /H /Y
ROBOCOPY jsoncpp\include\json\ %DEST%\include\json\


git clone https://github.com/jupyter-xeus/cpp-terminal
PUSHD cpp-terminal
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -DCPPTERMINAL_ENABLE_TESTING=OFF -DCPPTERMINAL_BUILD_EXAMPLES=OFF -DCPPTERMINAL_ENABLE_DOCS=OFF
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DCPPTERMINAL_ENABLE_TESTING=OFF -DCPPTERMINAL_BUILD_EXAMPLES=OFF -DCPPTERMINAL_ENABLE_DOCS=OFF
cmake --build . --config Release --parallel
POPD
POPD 
XCOPY cpp-terminal\build\cpp-terminal\Release\cpp-terminal.lib %DEST%\lib\ /K /D /H /Y
XCOPY cpp-terminal\build\cpp-terminal\Debug\cpp-terminal.lib %DEST%\lib\cpp-terminald.* /K /D /H /Y
XCOPY cpp-terminal\build\cpp-terminal\private\Release\cpp-terminal-private.lib %DEST%\lib\ /K /D /H /Y
XCOPY cpp-terminal\build\cpp-terminal\private\Debug\cpp-terminal-private.lib %DEST%\lib\cpp-terminal-privated.* /K /D /H /Y
ROBOCOPY cpp-terminal\cpp-terminal\ %DEST%\include\cpp-terminal\ *.h*
ROBOCOPY cpp-terminal\cpp-terminal\private\ %DEST%\include\cpp-terminal\private\ *.h*

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
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
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

git clone https://github.com/yhirose/cpp-httplib
PUSHD cpp-httplib
git checkout v0.18.1
POPD
COPY cpp-httplib\httplib.h %DEST%\include\httplib.* /Y

git clone --recursive https://github.com/cpp-redis/cpp_redis
PUSHD cpp_redis
git checkout 4.3.1
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --config Release --parallel
POPD
POPD
COPY cpp_redis\build\lib\Debug\tacopie.lib %DEST%\lib\tacopied.lib /Y
COPY cpp_redis\build\lib\Debug\cpp_redis.lib %DEST%\lib\cpp_redisd.lib /Y
COPY cpp_redis\build\lib\Release\tacopie.lib %DEST%\lib\tacopie.lib /Y
COPY cpp_redis\build\lib\Release\cpp_redis.lib %DEST%\lib\cpp_redis.lib /Y
ROBOCOPY cpp_redis\includes\ %DEST%\include\ /E
ROBOCOPY cpp_redis\tacopie\includes\ %DEST%\include\ /E

git clone https://github.com/boostorg/boost
PUSHD boost
git checkout boost-1.84.0
git submodule update --init --recursive
CALL bootstrap.bat
CALL b2.exe headers
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -DCMAKE_CXX_FLAGS="/utf-8" -DCMAKE_CXX_FLAGS="/EHsc"
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DCMAKE_CXX_FLAGS="/utf-8" -DCMAKE_CXX_FLAGS="/EHsc"
cmake --build . --config Release --parallel
POPD
POPD
ROBOCOPY boost\boost\ %DEST%\include\boost\ /E
ROBOCOPY boost\build\stage\lib\Debug\ %DEST%\lib\boost\ /E *.lib
ROBOCOPY boost\build\stage\lib\Release\ %DEST%\lib\boost\ /E *.lib

POPD
rmdir /s /q modules
PAUSE