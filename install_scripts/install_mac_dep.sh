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

if [ -d "$DEP_BASEDIR" ]; then
    echo "Dependency directory already exists. Re-using this directory!"
else
    mkdir $DEP_BASEDIR
fi
TMP_BASEDIR=$DEP_BASEDIR"tmp_libsrc/"
LIBDIR=$DEP_BASEDIR"lib/"
mkdir $LIBDIR
mkdir $TMP_BASEDIR

#
# Compile and install fast ann
#
FASTANN_LIBDIR=$LIBDIR"fastann/"

if [ -d "$FASTANN_LIBDIR" ]; then
    echo "Skipping FASTANN library as it already exists at "$FASTANN_LIBDIR
else
    cd $TMP_BASEDIR
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
# Install homebrew package manager
#
which -s brew
if [[ $? != 0 ]] ; then
    # Install Homebrew
    /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
else
    brew update
fi

#
# Install gcc-6
#
brew install gcc@6

brew upgrade cmake

#
# Install other dependencies
#
brew install libpng libjpeg openmpi

#
# Install ImageMagick
#
IMAGEMAGICK_LIBDIR=$LIBDIR"imagemagick/"
if [ -d "$IMAGEMAGICK_LIBDIR" ]; then
    echo "Skipping ImageMagick library as it already exists at "$IMAGEMAGICK_LIBDIR
else
    cd $TMP_BASEDIR
    #wget https://www.imagemagick.org/download/releases/ImageMagick-6.9.8-4.tar.gz
    wget -O ImageMagick-6.9.8-4.tar.gz http://git.imagemagick.org/repos/ImageMagick/repository/archive.tar.gz?ref=6.9.8-4
    tar -zxvf ImageMagick-6.9.8-4.tar.gz
    cd ImageMagick-6.9.8-4-ed5b4e81a2e1cdcb2a95db7119f26266c700d837
    CC=gcc-6 CXX=g++-6 ./configure --prefix=$IMAGEMAGICK_LIBDIR --with-quantum-depth=16 --disable-dependency-tracking --with-x=yes --without-perl
    make -j8
    make install
fi

#
# Install Google Protobuf
#
PROTOBUF_LIBDIR=$LIBDIR"protobuf/"
if [ -d "$PROTOBUF_LIBDIR" ]; then
    echo "Skipping PROTOBUF library as it already exists at "$PROTOBUF_LIBDIR
else
    cd $TMP_BASEDIR
    wget https://github.com/google/protobuf/releases/download/v2.6.1/protobuf-2.6.1.tar.gz
    tar -zxvf protobuf-2.6.1.tar.gz
    cd protobuf-2.6.1
    CC=gcc-6 CXX=g++-6 ./configure --prefix=$PROTOBUF_LIBDIR
    make -j8
    make install
fi



#
# Install Boost
#
BOOST_LIBDIR=$LIBDIR"boost/"
if [ -d "$BOOST_LIBDIR" ]; then
    echo "Skipping BOOST library as it already exists at "$BOOST_LIBDIR
else
    cd $TMP_BASEDIR
    wget https://netcologne.dl.sourceforge.net/project/boost/boost/1.64.0/boost_1_64_0.tar.gz
    tar -zxvf boost_1_64_0.tar.gz
    cd boost_1_64_0
    ./bootstrap.sh --prefix=$BOOST_LIBDIR --with-toolset=gcc --with-libraries=filesystem,system,thread,date_time,chrono,atomic,timer
    sed -i.old 's/using gcc ;/using gcc : 6.3.0 : g++-6 ;/g' project-config.jam
    ./b2 --with-filesystem --with-system --with-thread --with-date_time --with-chrono --with-atomic --with-timer variant=release threading=multi toolset=gcc install
fi


#
# Dependencies for frontend
#

pip install cherrypy numpy pillow

# for jp_draw
pip install cython
brew install cairo
