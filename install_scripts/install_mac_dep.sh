DEP_BASEDIR=$(pwd)"/../dep/"

if [ -d "$DEP_BASEDIR" ]; then
    echo "Dependency directory already exists. Re-using this directory!"
else
    mkdir $DEP_BASEDIR
fi
TMPDIR=$DEP_BASEDIR"tmp/"
LIBDIR=$DEP_BASEDIR"lib/"
mkdir $LIBDIR

#
# Compile and install fast ann
#
FASTANN_LIBDIR=$LIBDIR"fastann/"

if [ -d "$FASTANN_LIBDIR" ]; then
    echo "Skipping FASTANN library as it already exists at "$FASTANN_LIBDIR
else
    mkdir $FASTANN_LIBDIR
    mkdir $TMPDIR
    cd $TMPDIR
    wget -O fastann.zip https://github.com/philbinj/fastann/archive/master.zip
    unzip fastann.zip
    cd fastann-master
    mkdir build
    cd build
    export PREFIX=$FASTANN_LIBDIR
    cmake ../
    make
    make install
    cd $DEP_BASEDIR
    rm -fr $TMPDIR
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
# Install ImageMagick
#
brew install imagemagick@6

#
# Install Google Protobuf
#
brew install protobuf@2.6

#
# Install Boost
#
brew install boost@1.60
