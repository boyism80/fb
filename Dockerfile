FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/sdk:8.0 AS data-build
WORKDIR /
COPY resources/table /app/resources/table
COPY tools/data-converter /app/tools/data-converter
WORKDIR /app/tools/data-converter
RUN dotnet publish -c Release /p:DefineConstants=DISABLED_TTY -o bin
WORKDIR /app/tools/data-converter/bin
RUN ./ExcelTableConverter --dir=/app/resources/table --lang="c++|c#"
RUN mv output /output


FROM --platform=$BUILDPLATFORM alpine:3.21.2
RUN apk add build-base
RUN apk add boost-dev
RUN apk add cmake
RUN apk add ncurses-dev
RUN apk add git

WORKDIR /app
RUN git clone https://github.com/open-source-parsers/jsoncpp
WORKDIR /app/jsoncpp
RUN git checkout 1.9.6
WORKDIR /app/jsoncpp/build
RUN cmake .. 
RUN cmake --build . --config Release --parallel
RUN make install

WORKDIR /app
RUN git clone https://github.com/alanxz/rabbitmq-c
WORKDIR /app/rabbitmq-c
RUN git checkout v0.14.0
WORKDIR /app/rabbitmq-c/build
RUN cmake .. -DENABLE_SSL_SUPPORT=OFF
RUN cmake --build . --config Release --parallel
RUN make install

WORKDIR /app
RUN git clone --recursive https://github.com/boyism80/lua
WORKDIR /app/lua/build
RUN cmake .. 
RUN cmake --build . --config Release --parallel
RUN make install

WORKDIR /app
RUN git clone https://github.com/intel/zlib
WORKDIR /app/zlib
RUN git checkout v1.2.9
WORKDIR /app/zlib/build
RUN cmake .. 
RUN cmake --build . --config Release --parallel
RUN make install

WORKDIR /app
RUN git clone https://github.com/boyism80/flatbuffers
WORKDIR /app/flatbuffers/build
RUN cmake .. -DFLATBUFFERS_BUILD_FLATC=OFF -DFLATBUFFERS_BUILD_TESTS=OFF
RUN cmake --build . --config Release --parallel
RUN make install

WORKDIR /app
RUN git clone https://github.com/yhirose/cpp-httplib
WORKDIR /app/cpp-httplib
RUN git checkout v0.18.1
WORKDIR /app/cpp-httplib/build
RUN cmake .. 
RUN cmake --build . --config Release --parallel
RUN make install

WORKDIR /app
RUN git clone https://github.com/microsoft/cpp-async
WORKDIR /app/cpp-async
RUN git checkout v1.1.0
RUN cp -r /app/cpp-async/include/async /usr/local/include/async

WORKDIR /app
RUN git clone --recursive https://github.com/cpp-redis/cpp_redis.git
WORKDIR /app/cpp_redis/tacopie
RUN git fetch origin pull/5/head:cmake-fixes && git checkout cmake-fixes
RUN sed -i 's/add_library(${PROJECT_NAME} SHARED #${tacopie_sources})/add_library(${PROJECT_NAME} ${tacopie_sources})/' CMakeLists.txt
WORKDIR ../
RUN sed -i 's/add_library(${PROJECT_NAME} SHARED #${cpp_redis_sources})/add_library(${PROJECT_NAME} ${cpp_redis_sources})/' CMakeLists.txt
RUN mkdir build
WORKDIR ./build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release
RUN cmake --build . --config Release --parallel
RUN make install

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

ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8