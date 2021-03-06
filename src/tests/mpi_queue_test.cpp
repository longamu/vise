/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.
*/


#include "mpi_queue.h"

int main(int argc, char* argv[]) {
    
    MPI_INIT_ENV
    
    mpiQueue_test();
    
    return 0;
    
}
