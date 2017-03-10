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
#include <stdio.h>
#include <vector>
#include <stdint.h>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "document_map.h"
#include "evaluator.h"
#include "fidx_v2_as_v1.h"
#include "iidx_v2_as_v1.h"
#include "par_queue.h"
#include "proto_db_file.h"
#include "spatial_verif.h"
#include "tfidf.h"


int main(int argc, char* argv[]) {
    
    std::string dsetPrefix= "oxc1_5k";
    std::string dsetFn= "/home/relja/Relja/Data/oxc1_5k/dset_oxc1_5k.db";
    std::string iidxFn= "/home/relja/Relja/Data/tmp/indexing_v2/iidx_oxc1_5k_hesaff_sift_hell_1000000_43.v2bin";
    std::string fidxFn= "/home/relja/Relja/Data/tmp/indexing_v2/fidx_oxc1_5k_hesaff_sift_hell_1000000_43.v2bin";
    std::string wghtFn= "/home/relja/Relja/Data/oxc1_5k/wght_oxc1_5k_hesaff_sift_hell_1000000_43.rr.bin";
    
    #if 0
    protoDbFile iidx(iidxFn);
    protoDbFile fidx(fidxFn);
    #else
    protoDbFile iidxFile(iidxFn);
    protoDbInRam iidx(iidxFile);
    protoDbFile fidxFile(fidxFn);
    protoDbInRam fidx(fidxFile);
    #endif
    
    documentMap docMap( dsetFn.c_str(), dsetPrefix.c_str() );
    iidxV2AsV1 iidx_obj( iidx, docMap.numDocs() );
    fidxV2AsV1 fidx_obj( fidx, docMap.numDocs() );
    
    tfidf tfidf_obj( &iidx_obj, &fidx_obj, wghtFn.c_str() );
    
    spatialVerif spatVer_obj( &tfidf_obj, &fidx_obj, NULL, false );
    
    std::string gt_fn=
        (boost::format(std::string(getenv("HOME"))+"/Relja/Code/relja_retrieval/temp/oxford_%s.gt.bin")
        % dsetPrefix ).str();
    
    evaluator eval_obj( gt_fn.c_str(), false );
    
    std::vector<double> APs;
    
    parQueue<evaluator::APresultType> evalParQueue_obj(true, 0);
    
    // tfidf
    double mAP_tfidf= eval_obj.computeMAP( tfidf_obj, APs, true, true, &evalParQueue_obj );
        
    // spatial verification
    double mAP_sp= eval_obj.computeMAP( spatVer_obj, APs, true, true, &evalParQueue_obj );
    
    if ( evalParQueue_obj.getRank()==0 ){
        printf("mAP_tfidf= %.4f\n", mAP_tfidf);
        printf("mAP_sp   = %.4f\n", mAP_sp);
    }
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
    
}
