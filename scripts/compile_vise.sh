#!/bin/sh

if [ "$#" -ne 1 ] || ! [ -d "$1" ]; then
  echo "Usage: $0 DEPENDENCY_FILES_LOCATION/" >&2
  exit 1
fi

DEP_BASEDIR=$1
SRCDIR=$(pwd)"/src/"
BUILDDIR=$(pwd)"/build/"
mkdir $BUILDDIR

#
# Compile pypar and dkmeans_relja
#
( cd $SRCDIR"external/dkmeans_relja/pypar_2.1.4_94/source" ;
  python setup.py build ;
  python setup.py install
)

( cd $SRCDIR"external/dkmeans_relja/" ;
  python setup.py build ;
  python setup.py install ;
)


CMAKE_BIN_DIR="${DEP_BASEDIR}lib/cmake/bin"
IMAGEMAGICK_LIBDIR="${DEP_BASEDIR}lib/imagemagick/"
BOOST_LIBDIR="${DEP_BASEDIR}lib/boost/"
FASTANN_LIBDIR="${DEP_BASEDIR}lib/fastann/"
PROTOBUF_LIBDIR="${DEP_BASEDIR}lib/protobuf/"

(export CMAKE_PREFIX_PATH=$IMAGEMAGICK_LIBDIR:$BOOST_LIBDIR:$FASTANN_LIBDIR:$PROTOBUF_LIBDIR; 
 cd $BUILDDIR; 
 "${CMAKE_BIN_DIR}/cmake" -DCMAKE_BUILD_TYPE=Release $SRCDIR;
 make -j8)

