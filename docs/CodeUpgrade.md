# Plan for Codebase Upgrade of VISE
The following areas of VISE codebase (inherited from  `relja_retrival` codebase) 
are planned to be upgraded as we move towards an open source release of VISE:

## Remove (or replace) external dependency `KMCode_relja`
 * Very difficult to debug and maintain
 * "a lot of memory leaks and bugs with accessing uninitialised values"
 * replace this dependency with [vlfeat](http://www.vlfeat.org/)
 * A quick way to improve code performance is to avoid writing regions and 
descriptors to hard disk and directly share the variables holding this data in 
KMCode_relja.
   * In `src/preprocessing/feat_standard.cpp`, the method 
`reg_KM_HessAff::getRegs()` writes regions to a file and `readRegions()` method 
reads it back into memory
   * instead of this, directly convert 
`src/external/KMCode_relja/descriptor/CornerDescriptor` to `std::vector<ellipse> &regions`
and pass the data via memory
   * see `src/external/KMCode_relja/exec/detect_points` line 411

## Replace fastann dependency
 * cannot be compiled for Microsoft Windows platform
 * replace with [flann](http://www.cs.ubc.ca/research/flann/)

## Multi-threaded clustering
 * At present, `../src/v2/indexing/compute_clusters.py` uses only a single thread
 * This can be improved if it can utilize multiple threads available in computer

## Documentation of Multi-threaded execution model employed by VISE
```
/home/tlm/dev/vise/src/util/par_queue.h
/home/tlm/dev/vise/src/util/thread_queue.h
```

