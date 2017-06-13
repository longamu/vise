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
BUILD_DIR=$(pwd)"/../build/"
VISE_ROOTDIR=$(pwd)"/../"
DEP_BASEDIR=$1

export VISE_SRC_DIR=$VISE_ROOTDIR
export VISE_DATA_DIR="/home/tlm/vise"
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
    cmake $VISE_ROOTDIR"src/"
    make -j8
fi
