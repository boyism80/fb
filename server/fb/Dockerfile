FROM --platform=$BUILDPLATFORM ubuntu:24.04

# Build type argument - Release as default for Dockerfile
ARG BUILD_TYPE=Release

# Pinned refs — keep in sync with tools/update-modules.bat
ARG JSONCPP_REF=1.9.6
ARG RABBITMQ_REF=v0.14.0
ARG ZLIB_REF=v1.2.9
ARG CPP_ASYNC_REF=v1.1.3
ARG BOOST_REF=boost-1.84.0

RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt-get install git gcc-14 g++-14 gdb make cmake libssl-dev libstdc++-14-dev wget -y
RUN ln -s -f /usr/bin/gcc-14 /usr/bin/gcc && ln -s -f /usr/bin/g++-14 /usr/bin/g++
RUN apt-get install libncurses5-dev libncursesw5-dev -y
RUN apt-get install libubsan1 -y


WORKDIR /app
RUN git clone --branch ${JSONCPP_REF} https://github.com/open-source-parsers/jsoncpp /app/jsoncpp
WORKDIR /app/jsoncpp/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --parallel 12
RUN make install

WORKDIR /app
RUN git clone --branch ${RABBITMQ_REF} https://github.com/alanxz/rabbitmq-c /app/rabbitmq-c
WORKDIR /app/rabbitmq-c/build
RUN cmake .. -DENABLE_SSL_SUPPORT=OFF
RUN cmake --build . --config Release --parallel 12
RUN make install

# No pinned ref in update-modules.bat; tracks fork default branch.
WORKDIR /app
RUN git clone --recursive https://github.com/boyism80/lua /app/lua
WORKDIR /app/lua/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --parallel 12
RUN make install

WORKDIR /app
RUN git clone --branch ${ZLIB_REF} https://github.com/intel/zlib /app/zlib
WORKDIR /app/zlib/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --parallel 12
RUN make install

# No pinned ref in update-modules.bat; tracks fork default branch.
WORKDIR /app
RUN git clone https://github.com/boyism80/flatbuffers /app/flatbuffers
WORKDIR /app/flatbuffers/build
RUN cmake .. -DFLATBUFFERS_BUILD_FLATC=OFF -DFLATBUFFERS_BUILD_TESTS=OFF
RUN cmake --build . --config Release --parallel 12
RUN make install

WORKDIR /app
RUN git clone --depth 1 --branch ${CPP_ASYNC_REF} https://github.com/boyism80/cpp-async /app/cpp-async
RUN cp -r /app/cpp-async/include/async /usr/local/include/async

# No pinned ref in update-modules.bat; tracks default branch.
WORKDIR /app
RUN git clone https://github.com/cjgdev/aho_corasick /app/aho_corasick
RUN cp -r /app/aho_corasick/src/aho_corasick /usr/local/include/aho_corasick

WORKDIR /app
RUN git clone --branch ${BOOST_REF} https://github.com/boostorg/boost /app/boost
WORKDIR /app/boost
RUN git submodule update --init --recursive
RUN ./bootstrap.sh
RUN ./b2
RUN ./b2 install

# fb.lib
WORKDIR /app
COPY include ./include
COPY server/fb ./fb
WORKDIR /app/fb/build
RUN cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
RUN cmake --build . --parallel 12
RUN make install

RUN rm -rf /app
WORKDIR /
RUN ldconfig

ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
