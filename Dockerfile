FROM mcr.microsoft.com/dotnet/sdk:8.0 AS data-build
WORKDIR /
COPY resources/table /app/resources/table
COPY tools/data-converter /app/tools/data-converter
WORKDIR /app/tools/data-converter
RUN dotnet publish -c Release -o bin
WORKDIR /app/tools/data-converter/bin
RUN ./ExcelTableConverter --dir=/app/resources/table --lang="c++|c#"
RUN mv output /output


FROM ubuntu:24.04

RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt-get install git gcc-14 g++-14 gdb make cmake libssl-dev libjsoncpp-dev libstdc++-14-dev libboost-program-options-dev wget -y
RUN ln -s -f /usr/bin/gcc-14 /usr/bin/gcc && ln -s -f /usr/bin/g++-14 /usr/bin/g++


COPY dependency/jsoncpp /app/dependency/jsoncpp
WORKDIR /app/dependency/jsoncpp/build
RUN cmake .. 
RUN cmake --build . --config Release --parallel
RUN make install

COPY dependency/cpp-terminal /app/dependency/cpp-terminal
WORKDIR /app/dependency/cpp-terminal/build
RUN cmake .. -DCPPTERMINAL_ENABLE_TESTING=OFF -DCPPTERMINAL_BUILD_EXAMPLES=OFF -DCPPTERMINAL_ENABLE_DOCS=OFF
RUN cmake --build . --config Release --parallel
RUN make install

COPY dependency/rabbitmq-c /app/dependency/rabbitmq-c
WORKDIR /app/dependency/rabbitmq-c/build
RUN cmake .. -DENABLE_SSL_SUPPORT=OFF
RUN cmake --build . --config Release --parallel
RUN make install

COPY dependency/lua /app/dependency/lua
WORKDIR /app/dependency/lua/build
RUN cmake .. 
RUN cmake --build . --config Release --parallel
RUN make install

COPY dependency/zlib /app/dependency/zlib
WORKDIR /app/dependency/zlib/build
RUN cmake .. 
RUN cmake --build . --config Release --parallel
RUN make install

COPY dependency/flatbuffers /app/dependency/flatbuffers
WORKDIR /app/dependency/flatbuffers/build
RUN cmake .. -DFLATBUFFERS_BUILD_FLATC=OFF -DFLATBUFFERS_BUILD_TESTS=OFF
RUN cmake --build . --config Release --parallel
RUN make install

COPY dependency/cpp-httplib /app/dependency/cpp-httplib
WORKDIR /app/dependency/cpp-httplib/build
RUN cmake .. 
RUN cmake --build . --config Release --parallel
RUN make install

COPY dependency/cpp-async/include/async /usr/local/include/async

COPY dependency/boost /app/dependency/boost
WORKDIR /app/dependency/boost
RUN ./bootstrap.sh --prefix=/usr/
RUN ./b2 --with-program_options
RUN ./b2 install
WORKDIR /
RUN rm -rf /app/boost

RUN git clone https://github.com/cpp-redis/cpp_redis.git
WORKDIR ./cpp_redis
RUN git submodule init && git submodule update
WORKDIR ./tacopie
RUN git fetch origin pull/5/head:cmake-fixes && git checkout cmake-fixes
RUN sed -i 's/add_library(${PROJECT_NAME} SHARED ${tacopie_sources})/add_library(${PROJECT_NAME} ${tacopie_sources})/' CMakeLists.txt
WORKDIR ../
RUN sed -i 's/add_library(${PROJECT_NAME} SHARED ${cpp_redis_sources})/add_library(${PROJECT_NAME} ${cpp_redis_sources})/' CMakeLists.txt
RUN mkdir build
WORKDIR ./build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release
RUN cmake --build . --config Release --parallel
RUN make install
WORKDIR /
RUN rm -rf cpp_redis

# fb.lib
WORKDIR /app
COPY include ./include
COPY lib ./lib
WORKDIR /app/lib/build
RUN cmake ..
RUN cmake --build . --config Release --parallel
RUN make install

RUN rm -rf /app
WORKDIR /fb/data
COPY --from=data-build /output .
WORKDIR /
RUN ldconfig

ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8