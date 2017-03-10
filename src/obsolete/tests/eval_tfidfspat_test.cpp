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

#include "util.h"
#include "par_queue.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "fidx_wrapper_jp_db5.h"
#include "iidx_wrapper_jp_gen.h"
#include "iidx_in_ram.h"
#include "fidx_in_ram.h"
#include "tfidf.h"
#include "spatial_verif.h"
#include "document_map.h"
#include "evaluator.h"


int main(int argc, char* argv[]) {
    
    MPI_INIT_ENV;
    
    std::string datapath= std::string(getenv("HOME"))+"/Relja/Data";
    char dset_fn[200], iidx_fn[200], fidx_fn[200], wght_fn[200];
    
    const char prefix[]= "oxc1_5k";
//     const char prefix[]= "ox100k";
    uint32_t k= 1000000;
    uint32_t seed= 43;
    bool useHellinger= true;
//     bool useHellinger= false;
    
    sprintf(dset_fn, "%s/%s/dset_%s.db", datapath.c_str(), prefix, prefix);
    if (!useHellinger){
        const char detdesc[]= "hesaff_sift";
        sprintf(iidx_fn, "%s/%s/iidx_%s_%s_%d_%d.bin", datapath.c_str(), prefix, prefix, detdesc, k, seed);
        sprintf(fidx_fn, "%s/%s/word_%s_%s_%d_%d.h5", datapath.c_str(), prefix, prefix, detdesc, k, seed);
        sprintf(wght_fn, "%s/%s/wght_%s_%s_%d_%d.rr.bin", datapath.c_str(), prefix, prefix, detdesc, k, seed);
    } else {
        const char detdesc[]= "hesaff_sift_hell";
        sprintf(iidx_fn, "%s/ox_exp/iidx_%s_%s_%d_%d.bin", datapath.c_str(), prefix, detdesc, k, seed);
        sprintf(fidx_fn, "%s/ox_exp/word_%s_%s_%d_%d.h5", datapath.c_str(), prefix, detdesc, k, seed);
        sprintf(wght_fn, "%s/ox_exp/wght_%s_%s_%d_%d.rr.bin", datapath.c_str(), prefix, detdesc, k, seed);
    }
    
    documentMap docMap( dset_fn, prefix );
    #if 0
    iidxWrapperJpGen<uint32_t> *iidx_obj= new iidxWrapperJpGen<uint32_t>( iidx_fn );
    #else
    invertedIndex *iidx_obj= new iidxInRam( iidx_fn );
    #endif
    
    #if 0
    fidxWrapperJpDb5_HardJP *fidx_obj= new fidxWrapperJpDb5_HardJP(fidx_fn, docMap);
    #else
    fidxInRam *fidx_obj= NULL;
    {
        fidxWrapperJpDb5_HardJP fidx_obj_orig( fidx_fn, docMap);
        fidx_obj= new fidxInRam(fidx_obj_orig);
    }
    #endif
    
    tfidf *tfidf_obj= NULL;
    
    if ( boost::filesystem::exists( wght_fn ) ){
        tfidf_obj= new tfidf( iidx_obj, fidx_obj, wght_fn );
    } else {
        tfidf_obj= new tfidf( iidx_obj, fidx_obj );
        tfidf_obj->saveToFile( wght_fn );
    }
    
    spatialVerif spatVer_obj( tfidf_obj, fidx_obj, NULL, false );
    
    std::string gt_fn=
        (boost::format(std::string(getenv("HOME"))+"/Relja/Code/relja_retrieval/temp/oxford_%s.gt.bin")
        % prefix ).str();
    
    evaluator *eval_obj= NULL;
    if ( boost::filesystem::exists( gt_fn.c_str() ) ){
        eval_obj= new evaluator( gt_fn.c_str(), false );
    } else {
        eval_obj= new evaluator( "/home/relja/Relja/Code/relja_retrieval/temp/oxford.gt", docMap, false );
        eval_obj->saveToFile( gt_fn.c_str() );
    }
    
    std::vector<double> APs;
    
    parQueue<evaluator::APresultType> evalParQueue_obj(true, 0);
    
    /**/
    // tfidf
    double mAP_tfidf= eval_obj->computeMAP( *tfidf_obj, APs, true, true, &evalParQueue_obj );
        
    // spatial verification
    double mAP_sp= eval_obj->computeMAP( spatVer_obj, APs, true, true, &evalParQueue_obj );
    
    if ( evalParQueue_obj.getRank()==0 ){
        printf("mAP_tfidf= %.4f\n", mAP_tfidf);
        printf("mAP_sp   = %.4f\n", mAP_sp);
    }
    /**/
    
    /*
    std::vector<indScorePair> queryRes;
    uint32_t queryID= 0;
    
    std::cout<< eval_obj->queries[queryID].xl <<" "<<eval_obj->queries[queryID].xu <<" "<<eval_obj->queries[queryID].yl <<" "<<eval_obj->queries[queryID].yu<<"\n";
    
//     tfidf_obj->internalQuery( eval_obj->queries[queryID], queryRes );
    spatVer_obj.internalQuery( eval_obj->queries[queryID], queryRes );
    
    for (int i=0; i<15; ++i)
        std::cout<< docMap.i2h(queryRes[i].first) <<" "<<queryRes[i].second<<"\n";
    */
    
    delete eval_obj;
    delete iidx_obj;
    delete fidx_obj;
    delete tfidf_obj;
    
}
