BUILD_DIR=$(pwd)"/../build/"
VISE_ROOTDIR=$(pwd)"/../"

if [ -d "$BUILD_DIR" ]; then
    rm -fr $BUILD_DIR
fi

mkdir $BUILD_DIR


FASTANN_LIBDIR=$VISE_ROOTDIR"dep/lib/fastann/"
IMAGEMAGICK_LIBDIR="/usr/local/Cellar/imagemagick@6/6.9.8-0/"
PROTOBUF_LIBDIR="/usr/local/Cellar/protobuf@2.6/2.6.0/"
BOOST_LIBDIR="/usr/local/Cellar/boost@1.60/1.60.0/"
VISE_CMAKE_PREFIX_PATH=$FASTANN_LIBDIR";"$IMAGEMAGICK_LIBDIR";"$PROTOBUF_LIBDIR

cd $BUILD_DIR
cmake -Wno-dev -DCMAKE_PREFIX_PATH=$VISE_CMAKE_PREFIX_PATH -DBOOST_ROOT=$BOOST_LIBDIR $VISE_ROOTDIR"src/"
