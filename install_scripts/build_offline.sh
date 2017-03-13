sudo apt-get install \
    openmpi-bin libopenmpi-dev

sudo apt-get install \
    libboost-mpi-dev

./build_backend.sh

echo "Now install dkmeans_relja (available in src/external/dkmeans_relja)"
echo "Optonal but recommended: install pypar"
