#!/bin/sh
cd /home/tlm/dev/vise/cmake_build
/home/tlm/deps/imcomp/lib/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="/home/tlm/deps/vise/lib" -DCMAKE_MODULE_PATH="/home/tlm/dev/vise/src/search_engine/relja_retrival/forbuild" ../
