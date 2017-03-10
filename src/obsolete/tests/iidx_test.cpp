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

#include "iidx_wrapper_jp_gen.h"

#include <stdint.h>

int main(){
    iidxWrapperJpGen<uint32_t> iidx( "/home/relja/Relja/Code/relja_retrieval/temp/iidx_oxc1_5k_hesaff_sift_1000000_43.bin" );
    iidx.test();
    printf("\n\tDone!\n");
    return 0;
}
