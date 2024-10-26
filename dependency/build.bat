git submodule update --recursive --remote

PUSHD jsoncpp
git checkout 1.9.6
git add .
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DJSONCPP_STATIC_WINDOWS_RUNTIME=ON -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
cmake --build . --target jsoncpp_static --config Debug
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DJSONCPP_STATIC_WINDOWS_RUNTIME=ON -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --target jsoncpp_static --config Release
POPD
POPD
XCOPY jsoncpp\build\lib\Debug\jsoncpp_static.lib lib\jsoncppd.* /K /D /H /Y
XCOPY jsoncpp\build\lib\Release\jsoncpp_static.lib lib\ /K /D /H /Y
ROBOCOPY jsoncpp\include\json\ include\jsoncpp\json\


PUSHD cpp-terminal
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -DCPPTERMINAL_ENABLE_TESTING=OFF -DCPPTERMINAL_BUILD_EXAMPLES=OFF -DCPPTERMINAL_ENABLE_DOCS=OFF
cmake --build . --config Debug
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DCPPTERMINAL_ENABLE_TESTING=OFF -DCPPTERMINAL_BUILD_EXAMPLES=OFF -DCPPTERMINAL_ENABLE_DOCS=OFF
cmake --build . --config Release
POPD
POPD 
XCOPY cpp-terminal\build\cpp-terminal\Release\cpp-terminal.lib lib\ /K /D /H /Y
XCOPY cpp-terminal\build\cpp-terminal\Debug\cpp-terminal.lib lib\cpp-terminald.* /K /D /H /Y
XCOPY cpp-terminal\build\cpp-terminal\private\Release\cpp-terminal-private.lib lib\ /K /D /H /Y
XCOPY cpp-terminal\build\cpp-terminal\private\Debug\cpp-terminal-private.lib lib\cpp-terminal-privated.* /K /D /H /Y
ROBOCOPY cpp-terminal\cpp-terminal\ include\cpp-terminal\ *.h*
ROBOCOPY cpp-terminal\cpp-terminal\private\ include\cpp-terminal\private\ *.h*

PUSHD rabbitmq-c
git checkout v0.14.0
git add .
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -DENABLE_SSL_SUPPORT=OFF
cmake --build . --target rabbitmq-static --config Debug
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DENABLE_SSL_SUPPORT=OFF
cmake --build . --target rabbitmq-static --config Release
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
cmake --build . --config Debug
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --config Release
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
cmake --build . --config Debug
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --config Release
POPD
POPD
XCOPY zlib\build\Debug\zlibd.lib lib\zlibd.* /K /D /H /Y
XCOPY zlib\build\Release\zlib.lib lib\zlib.* /K /D /H /Y
COPY zlib\zconf.h.included include\zlib\zconf.h /Y
ROBOCOPY zlib\ include\zlib\ *.h

PUSHD flatbuffers
git add .
if not exist build mkdir build
PUSHD build
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug -DFLATBUFFERS_BUILD_FLATC=OFF -DFLATBUFFERS_BUILD_TESTS=OFF -DFLATBUFFERS_INSTALL=OFF
cmake --build . --config Debug
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DFLATBUFFERS_BUILD_FLATC=OFF -DFLATBUFFERS_BUILD_TESTS=OFF -DFLATBUFFERS_INSTALL=OFF
cmake --build . --config Release
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
cmake --build . --config Debug
cmake .. -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build . --config Release
POPD
POPD
COPY cpp_redis\build\lib\Debug\tacopie.lib lib\tacopied.lib /Y
COPY cpp_redis\build\lib\Debug\cpp_redis.lib lib\cpp_redisd.lib /Y
COPY cpp_redis\build\lib\Release\tacopie.lib lib\tacopie.lib /Y
COPY cpp_redis\build\lib\Release\cpp_redis.lib lib\cpp_redis.lib /Y
ROBOCOPY cpp_redis\includes\ include\ /E
ROBOCOPY cpp_redis\tacopie\includes\ include\ /E
PAUSE