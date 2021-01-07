FROM ubuntu:18.04

RUN apt-get update -y && apt-get upgrade -y && apt-get install build-essential git wget zlib1g-dev -y

COPY root /
#RUN tar -xfvz root_v6.18.04.Linux-ubuntu18-x86_64-gcc7.4.tar.gz
#RUN root
#RUN . /root/bin/thisroot.sh

COPY cmake-3.16.4-Linux-x86_64.sh /
RUN chmod +x cmake-3.16.4-Linux-x86_64.sh
RUN ./cmake-3.16.4-Linux-x86_64.sh --skip-license

RUN git clone https://github.com/vetlewi/TDR2tree.git
WORKDIR /TDR2tree

RUN mkdir build && cd build && cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON && make
RUN TDR2tree --help