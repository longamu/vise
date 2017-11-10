#!/bin/sh

if [ "$#" -ne 1 ] || ! [ -d "$1" ]; then
  echo "Usage: $0 DEPENDENCY_FILES_LOCATION/" >&2
  exit 1
fi

# $1
# |- lib  
# |- tmp_libsrc
DEP_BASEDIR=$1
DEP_LIBDIR="${DEP_BASEDIR}lib/"
DEP_SRCDIR="${DEP_BASEDIR}tmp_libsrc"

#
# Compile and install fast ann
#
FASTANN_LIBDIR=$DEP_LIBDIR"fastann/"

if [ -d "$FASTANN_LIBDIR" ]; then
    echo "Skipping FASTANN library as it already exists at "$FASTANN_LIBDIR
else
    echo "Installing fastann at "$FASTANN_LIBDIR
    cd $DEP_SRCDIR
    wget -O fastann.zip https://github.com/philbinj/fastann/archive/master.zip
    unzip fastann.zip
    mv fastann-master fastann
    cd fastann
    mkdir build
    cd build
    export PREFIX=$FASTANN_LIBDIR
    cmake ../
    make
    make install
    #cd $DEP_BASEDIR
    #rm -fr ./fastann
fi

#
# Install ImageMagick
#
IMAGEMAGICK_LIBDIR=$DEP_LIBDIR"imagemagick/"
if [ -d "$IMAGEMAGICK_LIBDIR" ]; then
    echo "Skipping ImageMagick library as it already exists at "$IMAGEMAGICK_LIBDIR
else
    echo "Installing imagemagick at "$IMAGEMAGICK_LIBDIR
    cd $DEP_SRCDIR
    #wget https://www.imagemagick.org/download/releases/ImageMagick-6.9.8-4.tar.gz
    #wget -O ImageMagick-6.9.8-4.tar.gz http://git.imagemagick.org/repos/ImageMagick/repository/archive.tar.gz?ref=6.9.8-4
    #wget -O ImageMagick-7.0.5-6.tar.gz http://git.imagemagick.org/repos/ImageMagick/repository/archive.tar.gz?ref=7.0.5-6
    wget -O ImageMagick-6.9.8-8.tar.gz http://git.imagemagick.org/repos/ImageMagick/repository/archive.tar.gz?ref=6.9.8-8
    tar -zxvf ImageMagick-6.9.8-8.tar.gz
    cd ImageMagick-6.9.8-8-664f8d98f090e088e241eec59da42d47373721a7
    ./configure --prefix=$IMAGEMAGICK_LIBDIR --with-quantum-depth=16 --disable-dependency-tracking --with-x=yes --without-perl
    make -j8
    make install
fi


#
# Install Boost
#
BOOST_DIR=$DEP_LIBDIR"boost/"
if [ -d "$BOOST_DIR" ]; then
    echo "Skipping BOOST library as it already exists at "$BOOST_DIR
else
    echo "Installing boost at "$BOOST_DIR
    cd $DEP_SRCDIR
    wget https://netcologne.dl.sourceforge.net/project/boost/boost/1.64.0/boost_1_64_0.tar.gz
    tar -zxvf boost_1_64_0.tar.gz
    cd boost_1_64_0
    ./bootstrap.sh --prefix=$BOOST_DIR --with-libraries=filesystem,system,thread,date_time,chrono,atomic,timer
    ./b2 --with-filesystem --with-system --with-thread --with-date_time --with-chrono --with-atomic --with-timer variant=release threading=multi install
fi


#
# Install cmake 3.8.0
#
CMAKE_DIR=$DEP_LIBDIR"cmake/"
if [ -d "$CMAKE_DIR" ]; then
  echo "Skipping CMAKE as it already exists at "$CMAKE_DIR
else
  echo "Installing cmake at "$CMAKE_DIR
  cd $DEP_SRCDIR
  wget https://cmake.org/files/v3.8/cmake-3.8.0.tar.gz
  tar -zxvf cmake-3.8.0.tar.gz
  cd cmake-3.8.0
  ./bootstrap --prefix=$CMAKE_DIR
  make -j8
  make install
fi

#
# Install Google Protobuf
#
PROTOBUF_DIR=$DEP_LIBDIR"protobuf/"
if [ -d "$PROTOBUF_DIR" ]; then
  echo "Skipping PROTOBUF library as it already exists at "$PROTOBUF_DIR
else
  echo "Installing protobuf at "$PROTOBUF_DIR
  cd $DEP_SRCDIR
  wget https://github.com/google/protobuf/releases/download/v2.6.1/protobuf-2.6.1.tar.gz
  tar -zxvf protobuf-2.6.1.tar.gz
  cd protobuf-2.6.1
  ./configure --prefix=$PROTOBUF_DIR
  make -j8
  make install
fi

## Run as root
# yum install cairo-devel.x86_64 cairo.x86_64
#
