DEP_BASEDIR=$(pwd)"/../dep/"

if [ -d "$DEP_BASEDIR" ]; then
    rm -fr $DEP_BASEDIR
fi

mkdir $DEP_BASEDIR
TMPDIR=$DEP_BASEDIR"tmp/"
LIBDIR=$DEP_BASEDIR"lib/"

mkdir $LIBDIR

# Compile and install fast ann
FASTANN_LIBDIR=$LIBDIR"fastann/"
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
rm -fr $TMPDIR
