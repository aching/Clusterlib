#!/bin/sh

# Example configure environment for Yahoo! to get all components built.
export CXX="/usr/releng/tools/gcc/4.1.1/bin/g++"
export CXXFLAGS="-static"
export CPPFLAGS="-I/usr/releng/external/boost/1.39.0_gcc411/include/boost-1_39 -I/usr/releng/internal/zookeeper/zookeeper-1.0.0/include -I/usr/releng/external/log4cxx/0.10.0_gcc411/include -I. -I/usr/releng/external/libmicrohttpd/0.4.0_gcc411/include -I/usr/releng/external/apr/1.2.12/include/apr-1 -I/usr/releng/external/apr-util/1.2.12/include/apr-1 -I/usr/releng/external/mpich2/mpich2-latest/install/include -I/usr/releng/external/cppunit/1.12.0_gcc411/include"
export LDFLAGS="-L/usr/releng/external/boost/1.39.0_gcc411/lib -L/usr/releng/internal/zookeeper/zookeeper-1.0.0/lib -L/usr/releng/external/log4cxx/0.10.0_gcc411/lib -L/usr/releng/external/libmicrohttpd/0.4.0_gcc411/lib -L/usr/releng/external/apr/1.2.12/lib -L/usr/releng/external/apr-util/1.2.12/lib -L/usr/releng/external/mpich2/mpich2-latest/install/lib -L/usr/releng/external/cppunit/1.12.0_gcc411/lib -L/usr/local/lib/"
./configure --enable-static --with-zookeeper-jars-path=/usr/releng/internal/zookeeper/zookeeper-1.0.0/lib
