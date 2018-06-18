cd /home/tlm/dev/vise/cmake_build
/home/tlm/deps/imcomp/lib/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="/home/tlm/deps/vise/lib" -DCMAKE_MODULE_PATH="/home/tlm/dev/vise/src/search_engine/relja_retrival/forbuild" ../


/home/tlm/deps/imcomp/lib/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="/home/tlm/deps/imcomp/lib;/home/tlm/deps/vise/lib" -DCMAKE_MODULE_PATH="/home/tlm/dev/vise/src/search_engine/relja_retrival/forbuild" ../

#compile imagemagick q8

# compiling boost
./bootstrap.sh --prefix=$BOOST_LIBDIR --with-toolset=gcc --with-libraries=filesystem,system,thread,date_time,chrono,atomic,timer,mpi
add to project-config.jam: using mpi ;
./b2 --with-filesystem --with-system --with-thread --with-date_time --with-chrono --with-atomic --with-timer --with-mpi variant=release threading=multi toolset=gcc install


