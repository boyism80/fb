FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/sdk:8.0 AS data-build
WORKDIR /
COPY resources/table /app/resources/table
COPY tools/data-converter /app/tools/data-converter
WORKDIR /app/tools/data-converter
RUN dotnet publish -c Release /p:DefineConstants=DISABLED_TTY -o bin
WORKDIR /app/tools/data-converter/bin
RUN ./ExcelTableConverter --dir=/app/resources/table --lang="c++|c#" --dsl=/app/resources/table/dsl.json
RUN mv output /output


FROM --platform=$BUILDPLATFORM ubuntu:24.04

RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt-get install git gcc-14 g++-14 gdb make cmake libssl-dev libstdc++-14-dev wget -y
RUN ln -s -f /usr/bin/gcc-14 /usr/bin/gcc && ln -s -f /usr/bin/g++-14 /usr/bin/g++
RUN apt-get install libncurses5-dev libncursesw5-dev -y
RUN apt-get install libubsan1 -y


WORKDIR /app
RUN git clone https://github.com/open-source-parsers/jsoncpp
WORKDIR /app/jsoncpp
RUN git checkout 1.9.6
WORKDIR /app/jsoncpp/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. 
RUN cmake --build . --parallel
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
RUN cmake -DCMAKE_BUILD_TYPE=Release .. 
RUN cmake --build . --parallel
RUN make install

WORKDIR /app
RUN git clone https://github.com/intel/zlib
WORKDIR /app/zlib
RUN git checkout v1.2.9
WORKDIR /app/zlib/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. 
RUN cmake --build . --parallel
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
RUN cmake -DCMAKE_BUILD_TYPE=Release .. 
RUN cmake --build . --parallel
RUN make install

WORKDIR /app
RUN git clone https://github.com/microsoft/cpp-async
WORKDIR /app/cpp-async
RUN git checkout v1.1.0
RUN cp -r /app/cpp-async/include/async /usr/local/include/async

WORKDIR /app
RUN git clone https://github.com/boostorg/boost
WORKDIR /app/boost
RUN git checkout boost-1.84.0
RUN git submodule update --init --recursive
RUN ./bootstrap.sh
RUN ./b2
RUN ./b2 install

WORKDIR /app
RUN git clone https://github.com/redis/hiredis.git
WORKDIR /app/hiredis
RUN git checkout v1.2.0
WORKDIR /app/hiredis/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. 
RUN cmake --build . --parallel
RUN make install

WORKDIR /app
RUN git clone https://github.com/p-ranav/indicators
WORKDIR /app/indicators
RUN git checkout v2.3
WORKDIR /app/indicators/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. 
RUN cmake --build . --parallel
RUN make install

# fb.lib
WORKDIR /app
COPY include ./include
COPY lib ./lib
WORKDIR /app/lib/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. 
RUN cmake --build . --parallel
RUN make install

RUN rm -rf /app
WORKDIR /fb/data
COPY --from=data-build /output .
WORKDIR /
RUN ldconfig

ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8