BUILD_DIR=$(pwd)"/../build/"
VISE_ROOTDIR=$(pwd)"/../"

FASTANN_LIBDIR=$VISE_ROOTDIR"dep/lib/fastann/"
IMAGEMAGICK_LIBDIR=$VISE_ROOTDIR"dep/lib/imagemagick/"
PROTOBUF_LIBDIR=$VISE_ROOTDIR"dep/lib/protobuf/"
BOOST_LIBDIR=$VISE_ROOTDIR"dep/lib/boost/"
VISE_CMAKE_PREFIX_PATH=$FASTANN_LIBDIR";"$IMAGEMAGICK_LIBDIR";"$PROTOBUF_LIBDIR

if [ -d "$BUILD_DIR" ]; then
    #rm -fr $BUILD_DIR
    echo "Build dir already exists! "$BUILD_DIR
    exit
else
    mkdir $BUILD_DIR
    cd $BUILD_DIR
    CC=gcc-5 CXX=g++-5 cmake -Wno-dev -DCMAKE_PREFIX_PATH=$VISE_CMAKE_PREFIX_PATH -DBOOST_ROOT=$BOOST_LIBDIR $VISE_ROOTDIR"src/"
    make -j8 api_v2

    # export environment variables
    if [[ ! -f ~/.bash_profile ]]; then
        touch ~/.bash_profile
    fi

    if grep -q "export VISE_ROOTDIR" ~/.bash_profile; then
        echo "~/.bash_profile already contains VISE environment variable!"
    else
        echo export DYLD_FALLBACK_LIBRARY_PATH=$FASTANN_LIBDIR"lib/:"$PROTOBUF_LIBDIR"lib/:"$BOOST_LIBDIR"lib/" >> ~/.bash_profile
        echo export VISE_ROOTDIR=$VISE_ROOTDIR >> ~/.bash_profile
        echo export VISE_BUILDDIR=$BUILD_DIR >> ~/.bash_profile
        source ~/.bash_profile
    fi
fi
