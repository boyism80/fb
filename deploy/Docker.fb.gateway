FROM ubuntu:18.04

RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt-get install software-properties-common -y
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y
RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt-get install libjsoncpp-dev -y
RUN apt-get install libmysqlclient-dev -y
RUN apt install gcc-10 gcc-10-base gcc-10-doc -y
RUN apt install libstdc++-10-dev libstdc++-10-doc -y 