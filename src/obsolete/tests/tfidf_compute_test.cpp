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

#include <iostream>

#include "inverted_index.h"
#include "iidx_wrapper_jp_gen.h"
#include "tfidf.h"
#include "iidx_in_ram.h"
#include "timing.h"


int main(){
    
    const char iidxFn[]= "/home/relja/Relja/Data/oxc1_5k/iidx_oxc1_5k_hesaff_sift_1000000_43.bin";
    invertedIndex *iidx_orig= NULL;
    invertedIndex *iidx_obj= NULL;
    
    bool inRam= true;
    bool threadSafe= false;
    
    if (!inRam || threadSafe){
        iidx_orig= new iidxWrapperJpGen<uint32_t>( iidxFn );
    }
    
    if (inRam){
        if (threadSafe){
            iidx_obj= new iidxInRam( *iidx_orig );
        }
        else
            iidx_obj= new iidxInRam( iidxFn );
    }
    else
        iidx_obj= iidx_orig;
    
//     char fn[] = "/home/relja/Relja/Code/relja_retrieval/temp/tfidf.ser.bin";
    
    forwardIndex *forwardIndex_obj= NULL;
    #if 1
        tfidf tfidf_obj( iidx_obj, forwardIndex_obj );
//         tfidf_obj.saveToFile( fn );
    #else
        tfidf tfidf_obj( iidx_obj, forwardIndex_obj, fn );
    #endif
    
    for (int i=0; i<4; ++i){
        std::cout<< tfidf_obj.idf[i]<<" ";
    }
    std::cout<<"\n";
    for (int i=0; i<4; ++i){
        std::cout<< tfidf_obj.docL2[i]<<" ";
    }
    std::cout<<"\n";
    
    delete iidx_obj;
    if (threadSafe)
        delete iidx_orig;
    
}
