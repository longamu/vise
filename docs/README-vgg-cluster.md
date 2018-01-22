## Compiling in cluster
```
qrsh -pe mpislots 1
for modname in apps/cmake/2.8.9/gcc-4.4.6 libs/boost/1.51.0/gcc-4.4.6+openmpi-1.6.3 libs/gcc/system mpi/openmpi/1.6.3/gcc-4.4.6 ; do module add $modname; done
for modname in apps/python/2.7.3/gcc­-4.4.6 compilers/gcc/system libs/numpy/1.6.2/gcc-­4.4.6+python-­2.7.3+atlas­-3.10.0+westmere ; do module add $modname; done

for modname in apps/python/2.7.3/gcc-4.4.6 compilers/gcc/system libs/numpy/1.6.2/gcc-4.4.6+python-2.7.3+atlas-3.10.0+westmere ; do module add $modname; done                                                
for modname in apps/cmake/2.8.9/gcc-4.4.6 libs/boost/1.51.0/gcc-4.4.6+openmpi-1.6.3 libs/gcc/system mpi/openmpi/1.6.3/gcc-4.4.6 ; do module add $modname; done

source /etc/profile.d/modules.sh 
for modname in apps/cmake/2.8.9/gcc­-4.4.6 apps/python/2.7.3/gcc­-4.4.6 compilers/gcc/system libs/boost/1.51.0/gcc-­4.4.6+openmpi­-1.6.3 libs/gcc/system libs/numpy/1.6.2/gcc-­4.4.6+python-­2.7.3+atlas­-3.10.0+westmere mpi/openmpi/1.6.3/gcc-­4.4.6; 
do 
  module add $modname; 
done 

export PYTHONPATH=$PYTHONPATH:/users/adutta/build_deps/python-libs/lib64/python2.6/:/users/adutta/build_deps/python-libs/lib64/python2.6/site-packages/

# compile fastann (custom version)
# if online version is compiled, we get "Illegal Instruction" error when running trainAssign
qrsh -pe mpislots 1
for modname in apps/cmake/2.8.9/gcc-4.4.6 libs/boost/1.51.0/gcc-4.4.6+openmpi-1.6.3 libs/gcc/system mpi/openmpi/1.6.3/gcc-4.4.6 ; do module add $modname; done
export PREFIX=/users/adutta/build_deps/lib/fastann
cd /users/adutta/build_deps/tmp_libsrc/fastann/
mkdir build
cmake ../src
make
make install

# compile pypar (custom version)
cd /users/adutta/dev/relja_retrival/deps/pypar_2.1.4_94/source
python setup.py build 
python setup.py install --prefix=/users/adutta/build_deps/python-libs
export PYTHONPATH=/users/adutta/build_deps/python-libs/lib64/python2.6/site-packages/
mpirun -np 3 python -c "import pypar; pypar.finalize();" 

# compile dkmeans_relja
cd /users/adutta/dev/relja_retrival/code/src/external/dkmeans_relja
python setup.py build 
python setup.py install --prefix=/users/adutta/build_deps/python-libs

ln -s /opt/gridware/pkg/mpi/openmpi/1.6.3/gcc-4.4.6/lib/libmpi.so /users/adutta/build_deps/lib/openmpi/libmpi.so.0
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/users/adutta/build_deps/lib/openmpi
```

## index_dataset.sh

```
#!/bin/sh                                                                                                                                                                                                   
source /etc/profile.d/modules.sh

for modname in apps/cmake/2.8.9/gcc-4.4.6 apps/python/2.7.3/gcc-4.4.6 compilers/gcc/system libs/boost/1.51.0/gcc-4.4.6+openmpi-1.6.3 libs/gcc/system libs/numpy/1.6.2/gcc-4.4.6+python-2.7.3+atlas-3.10.0+w\
estmere mpi/openmpi/1.6.3/gcc-4.4.6;
do
  module add $modname;
done

export PYTHONPATH=$PYTHONPATH:/users/adutta/build_deps/python-libs/lib64/python2.6/:/users/adutta/build_deps/python-libs/lib64/python2.6/site-packages/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/users/adutta/build_deps/lib/openmpi

#export CMAKE_PREFIX_PATH=/users/adutta/build_deps/lib/imagemagick:/users/adutta/build_deps/lib/boost:/users/adutta/build_deps/lib/fastann:/users/adutta/build_deps/lib/protobuf                            
export CMAKE_PREFIX_PATH=/users/adutta/build_deps/lib/imagemagick:/users/adutta/build_deps/lib/fastann:/users/adutta/build_deps/lib/protobuf

cd /users/adutta/dev/relja_retrival/code/build

#dsetname=cameroon                                                                                                                                                                                          
dsetname=ox5k_test
NSLOTS=8

echo "Command: mpirun -np $NSLOTS v2/indexing/compute_index_v2 trainDescs ${dsetname}"
mpirun -np $NSLOTS v2/indexing/compute_index_v2 trainDescs ${dsetname}

echo "Command: mpirun -np $NSLOTS python ../src/v2/indexing/compute_clusters.py ${dsetname}"
mpirun -np $NSLOTS python ../src/v2/indexing/compute_clusters.py ${dsetname}

echo "Command: mpirun -np $NSLOTS v2/indexing/compute_index_v2 trainAssign ${dsetname}"
mpirun -np $NSLOTS v2/indexing/compute_index_v2 trainAssign ${dsetname}

echo "Command: mpirun -np $NSLOTS v2/indexing/compute_index_v2 trainHamm ${dsetname}"
mpirun -np $NSLOTS v2/indexing/compute_index_v2 trainHamm ${dsetname}

echo "Command: mpirun -np $NSLOTS v2/indexing/compute_index_v2 index ${dsetname}"
mpirun -np $NSLOTS v2/indexing/compute_index_v2 index ${dsetname}
mpirun -np 4 v2/indexing/compute_index_v2 index ox5k
```

## Debugging
```
cd /users/adutta/dev/relja_retrival/code/build
source /users/adutta/dev/relja_retrival/code/scripts/load_rr_indexing_env_titan.sh
```

## Running jobs in cluster
```
qsub -pe mpislots 8 /users/adutta/dev/relja_retrival/code/scripts/index_dset.sh
qsub -M adutta@robots.ox.ac.uk -pe mpislots 110 -m ea /users/adutta/dev/relja_retrival/code/scripts/index_dset.sh
```
