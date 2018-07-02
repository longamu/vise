#!/bin/sh

cd /home/tlm/dev/vise/
mkdir cmake_build
/home/tlm/deps/vise/lib/bin/cmake -DCMAKE_INSTALL_PREFIX=/home/tlm/deps/vise/lib ../

make -j 8

../bin/vise_server 0.0.0.0 10001 4 /home/tlm/dev/vise/asset /home/tlm/mydata/vise




