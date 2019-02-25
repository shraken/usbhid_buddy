FROM ubuntu:16.04

MAINTAINER Nicholas Shrake: 0.1

RUN apt-get update

RUN apt-get install -y build-essential 
RUN apt-get install -y autoconf 
RUN apt-get install -y automake 
RUN apt-get install -y python 
RUN apt-get install -y python-pip
RUN apt-get install -y cmake 
RUN apt-get install -y sdcc 
RUN apt-get install -y srecord 
RUN apt-get install -y doxygen 
RUN apt-get install -y graphviz 
RUN apt-get install -y swig

RUN apt-get update
RUN apt-get install -y libudev-dev 
RUN apt-get install -y libusb-1.0-0-dev

RUN pip install --upgrade pip
RUN pip install IntelHex
RUN pip install pyyaml
RUN pip install cpp-coveralls
RUN pip install gcovr