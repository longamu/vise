################################################################################
# dependencies installations in juno (40 core vgg server)
#
# Abhishek Dutta <adutta@robots.ox.ac.uk>
# 5 July 2018
################################################################################

# cmake
./configure --prefix=/ssd/adutta/dep/common/lib

#openmpi
wget https://download.open-mpi.org/release/open-mpi/v3.1/openmpi-3.1.1.tar.gz
tar -zxvf openmpi-3.1.1.tar.gz
./configure --prefix=/ssd/adutta/dep/common/lib

# imagemagick
./configure --prefix=/ssd/adutta/dep/common/lib -enable-hdri=no --with-quantum-depth=8 --disable-dependency-tracking --with-x=no --without-perl

# boost
./bootstrap.sh --prefix=/ssd/adutta/dep/common/lib --with-toolset=gcc --with-libraries=filesystem,system,thread,date_time,chrono,atomic,timer,mpi
echo "using mpi : /ssd/adutta/dep/common/lib/bin/mpic++ ;" >> project-config.jam
./b2 --with-filesystem --with-system --with-thread --with-date_time --with-chrono --with-atomic --with-timer --with-mpi variant=release threading=multi toolset=gcc install


# eigen
/ssd/adutta/dep/common/lib/bin/cmake -DCMAKE_INSTALL_PREFIX=/ssd/adutta/dep/common/lib ../
