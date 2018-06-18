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
#include <vector>
#include <stdint.h>

#include "par_queue.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "fidx_wrapper_jp_db5.h"
// #include "iidx_wrapper_jp_gen.h"
// #include "tfidf.h"
#include "document_map.h"
#include "spatial_verif.h"
#include "query.h"
#include "register_images.h"



int main(int argc, char* argv[]) {
    
    MPI_INIT_ENV;
    
    const char datapath[]= "/home/relja/Relja/Data/Ballads/colour";
    const char prefix[]= "ballads";
    const char detdesc[]= "hesaff_sift_hell";
    uint32_t k= 100000;
    uint32_t seed= 43;
    
    std::string dset_fn, iidx_fn, fidx_fn, wght_fn;
    
    dset_fn= (boost::format("%s/dset_%s.db")
              % datapath % prefix).str();
    iidx_fn= (boost::format("%s/iidx_%s_%s_%d_%d.bin")
              % datapath % prefix % detdesc % k % seed ).str();
    fidx_fn= (boost::format("%s/word_%s_%s_%d_%d.h5")
              % datapath % prefix % detdesc % k % seed ).str();
    wght_fn= (boost::format("%s/wght_%s_%s_%d_%d.rr.bin")
              % datapath % prefix % detdesc % k % seed ).str();
    
    documentMap docMap( dset_fn.c_str(), prefix, "/users/relja/Relja/Databases/Ballads/", "/home/relja/Relja/Databases/Ballads/highres/" );
    documentMapAsDatasetAbs dsetV2Wrapper(docMap);
    fidxWrapperJpDb5 *fidx_obj= new fidxWrapperJpDb5_HardJP( fidx_fn.c_str(), docMap);
    
    spatialVerif spatVer_obj( NULL, fidx_obj, NULL, false );
    
    
    //------------
    
    uint32_t docID1= docMap.h2i( "6c521c5c0cfe377f4af0e295c871b819b74e75ffc8af03aa9f27563f2e42e56c" );
    uint32_t docID2= docMap.h2i( "bff3909e0833a06d1d228e121b05ed76c47c6d157f21f223f8dd9b68aa840a9e" );
//     uint32_t docID2= docMap.h2i( "171e15f440c43cc61da3b714d7a625ba4d6d871950b4ae5cc44de81be5f2ac08" );
//     uint32_t docID2= docMap.h2i( "997b93079cf52ed7027e2e3ff985702dba68d6a98219eaaced65944717677846" );
    
    std::cout<<docMap.getFn( docID1 )<<"\n";
    std::cout<<docMap.getFn( docID2 )<<"\n";
    
    const char outFn1[]= "/home/relja/Relja/Code/relja_retrieval/temp/register/out/im1.jpg";
    const char outFn2[]= "/home/relja/Relja/Code/relja_retrieval/temp/register/out/im2.jpg";
    const char outFn2t[]= "/home/relja/Relja/Code/relja_retrieval/temp/register/out/im2t.jpg";
    
    query query_obj(docID1, true, "", 135, 371, 244, 389 );
    
    registerImages::registerFromQuery( query_obj, "", docID2, dsetV2Wrapper, spatVer_obj, outFn1, outFn2, outFn2t,
    "/home/relja/Relja/Code/relja_retrieval/temp/register/out/bib0101-M_L.jpg",
    "/home/relja/Relja/Code/relja_retrieval/temp/register/out/bib0033-M_L.jpg" );
    
    
    delete fidx_obj;
    
}
