#
# We assume the following directory tree
# $HOME/vgg/vise/
#           |- vise_dep
#              |- lib/
#              |- tmp_libsrc/
#           |- vise_src/vise/
#              |- vise
#                 |- src/
#                 |- install_scripts/
#                 |- ...
#           |- vise_data
#              |- log/
#              |- search_engines/
#
DEP_BASEDIR=$(pwd)"/../../../vise_dep/"

BUILD_DIR=$(pwd)"/../build/"
VISE_ROOTDIR=$(pwd)"/../"

FASTANN_LIBDIR=$VISE_ROOTDIR"../../vise_dep/lib/fastann/"
IMAGEMAGICK_LIBDIR=$VISE_ROOTDIR"../../vise_dep/lib/imagemagick/"
PROTOBUF_LIBDIR=$VISE_ROOTDIR"../../vise_dep/lib/protobuf/"
BOOST_LIBDIR=$VISE_ROOTDIR"../../vise_dep/lib/boost/"
LOCAL_LIB_DIR="/usr/local/lib"
VISE_CMAKE_PREFIX_PATH=$FASTANN_LIBDIR";"$IMAGEMAGICK_LIBDIR";"$PROTOBUF_LIBDIR

# export environment variables
if [[ ! -f ~/.bash_profile ]]; then
    touch ~/.bash_profile
fi

if grep -q "export VISE_ROOTDIR" ~/.bash_profile; then
    echo "~/.bash_profile already contains VISE environment variable!"
else
    echo export DYLD_FALLBACK_LIBRARY_PATH=$FASTANN_LIBDIR"lib/:"$PROTOBUF_LIBDIR"lib/:"$BOOST_LIBDIR"lib/:"$LOCAL_LIB_DIR >> ~/.bash_profile
    echo export VISE_ROOTDIR=$VISE_ROOTDIR >> ~/.bash_profile
    echo export VISE_BUILDDIR=$BUILD_DIR >> ~/.bash_profile
    source ~/.bash_profile
    echo "Saved environment variable to ~/.bash_profile"
fi

source ~/.bash_profile

#
# Compile pypar and dkmeans_relja
#
( cd $VISE_ROOTDIR"src/external/dkmeans_relja/pypar_2.1.4_94/source" ;
  python setup.py build ;
  sudo python setup.py install
)
( cd $VISE_ROOTDIR"src/external/dkmeans_relja/" ;
  python setup.py build ;
  sudo python setup.py install ;
)

if [ -d "$BUILD_DIR" ]; then
    #rm -fr $BUILD_DIR
    echo "Build dir already exists! "$BUILD_DIR
    exit
else
    mkdir $BUILD_DIR
    cd $BUILD_DIR
    CC=gcc-6 CXX=g++-6 cmake -Wno-dev -DCMAKE_PREFIX_PATH=$VISE_CMAKE_PREFIX_PATH -DBOOST_ROOT=$BOOST_LIBDIR $VISE_ROOTDIR"src/"
    make -j8 all
fi
