# Plan for Codebase Upgrade of VISE
The following areas of VISE codebase (inherited from  `relja_retrival` codebase) 
are planned to be upgraded as we move towards an open source release of VISE:

## Remove (or replace) external dependency `KMCode_relja`
 * Very difficult to debug and maintain
 * "a lot of memory leaks and bugs with accessing uninitialised values"

## Documentation of Multi-threaded execution model employed by VISE
```
/home/tlm/dev/vise/src/util/par_queue.h
/home/tlm/dev/vise/src/util/thread_queue.h
```

