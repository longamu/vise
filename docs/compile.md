cd /home/tlm/dev/vise/cmake_build
/home/tlm/deps/imcomp/lib/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="/home/tlm/deps/vise/lib" -DCMAKE_MODULE_PATH="/home/tlm/dev/vise/src/search_engine/relja_retrival/forbuild" ../

/home/tlm/deps/imcomp/lib/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="/home/tlm/deps/vise/lib" -DCMAKE_MODULE_PATH="/home/tlm/dev/vise/src/search_engine/relja_retrival/forbuild" ../
make -j 8 && ../bin/vise_server 0.0.0.0 9973 4 /home/tlm/dev/vise/asset /home/tlm/tmp/vise



#compile imagemagick q8

# compiling boost
./bootstrap.sh --prefix=/home/tlm/deps/vise/lib --with-toolset=gcc --with-libraries=filesystem,system,thread,date_time,chrono,atomic,timer,mpi,log

****IMPORTANT ***
echo "using mpi ;" >> project-config.jam

./b2 --with-filesystem --with-system --with-thread --with-date_time --with-chrono --with-atomic --with-timer --with-mpi --with-log variant=release threading=multi toolset=gcc install

./b2 --clean
