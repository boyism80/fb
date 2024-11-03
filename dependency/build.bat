git submodule update --init --recursive

PUSHD jsoncpp
git checkout 1.9.6
git add .
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DJSONCPP_STATIC_WINDOWS_RUNTIME=ON -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --target jsoncpp_static --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DJSONCPP_STATIC_WINDOWS_RUNTIME=ON -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --target jsoncpp_static --config Release --parallel
POPD
POPD
XCOPY jsoncpp\build\lib\Debug\jsoncpp_static.lib lib\jsoncppd.* /K /D /H /Y
XCOPY jsoncpp\build\lib\Release\jsoncpp_static.lib lib\ /K /D /H /Y
ROBOCOPY jsoncpp\include\json\ include\jsoncpp\json\

PUSHD rabbitmq-c
git checkout v0.14.0
git add .
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -DENABLE_SSL_SUPPORT=OFF
cmake --build . --target rabbitmq-static --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DENABLE_SSL_SUPPORT=OFF
cmake --build . --target rabbitmq-static --config Release --parallel
POPD
POPD
XCOPY rabbitmq-c\build\librabbitmq\Debug\librabbitmq.4.lib lib\librabbitmq.4d.* /K /D /H /Y
XCOPY rabbitmq-c\build\librabbitmq\Release\librabbitmq.4.lib lib\librabbitmq.4.* /K /D /H /Y
ROBOCOPY rabbitmq-c\include\rabbitmq-c\ include\rabbitmq-c\ *.h*
ROBOCOPY rabbitmq-c\build\include\rabbitmq-c\ include\rabbitmq-c\ *.h*

PUSHD lua
git add .
PUSHD upstream
git submodule init
git submodule update
POPD
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --config Release --parallel
POPD
POPD
XCOPY lua\build\Debug\lua.lib lib\luad.* /K /D /H /Y
XCOPY lua\build\Release\lua.lib lib\lua.* /K /D /H /Y
ROBOCOPY lua\upstream\ include\lua\ *.h*

PUSHD zlib
git checkout v1.2.9
git add .
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --config Release --parallel
POPD
POPD
XCOPY zlib\build\Debug\zlibstaticd.lib lib\zlibd.* /K /D /H /Y
XCOPY zlib\build\Release\zlibstatic.lib lib\zlib.* /K /D /H /Y
COPY zlib\zconf.h.included include\zlib\zconf.h /Y
ROBOCOPY zlib\ include\zlib\ *.h

PUSHD flatbuffers
git add .
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -DFLATBUFFERS_BUILD_FLATC=OFF -DFLATBUFFERS_BUILD_TESTS=OFF -DFLATBUFFERS_INSTALL=OFF
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DFLATBUFFERS_BUILD_FLATC=OFF -DFLATBUFFERS_BUILD_TESTS=OFF -DFLATBUFFERS_INSTALL=OFF
cmake --build . --config Release --parallel
POPD
POPD
XCOPY flatbuffers\build\Debug\flatbuffers.lib lib\flatbuffersd.* /K /D /H /Y
XCOPY flatbuffers\build\Release\flatbuffers.lib lib\flatbuffers.* /K /D /H /Y
ROBOCOPY flatbuffers\include\ include\ /E

ROBOCOPY cpp-async\include\async\ include\async\ *.h

PUSHD cpp-httplib
git checkout v0.18.1
POPD
COPY cpp-httplib\httplib.h include\httplib.* /Y

PUSHD cpp_redis
git checkout 4.3.1
git add .
PUSHD tacopie
git submodule init
git submodule update
POPD
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --config Debug --parallel
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --config Release --parallel
POPD
POPD
COPY cpp_redis\build\lib\Debug\tacopie.lib lib\tacopied.lib /Y
COPY cpp_redis\build\lib\Debug\cpp_redis.lib lib\cpp_redisd.lib /Y
COPY cpp_redis\build\lib\Release\tacopie.lib lib\tacopie.lib /Y
COPY cpp_redis\build\lib\Release\cpp_redis.lib lib\cpp_redis.lib /Y
ROBOCOPY cpp_redis\includes\ include\ /E
ROBOCOPY cpp_redis\tacopie\includes\ include\ /E

PUSHD boost
git checkout boost-1.84.0
git add .
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
ROBOCOPY boost\boost\ include\boost\ /E
ROBOCOPY boost\build\stage\lib\Debug\ lib\boost\ /E *.lib
ROBOCOPY boost\build\stage\lib\Release\ lib\boost\ /E *.lib

git submodule foreach git reset --hard
git submodule foreach git clean -fxd
PAUSE