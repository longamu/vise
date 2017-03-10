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
#include <string>

#include "util.h"
#include "par_queue.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "fidx_wrapper_jp_db5.h"
#include "iidx_wrapper_jp_gen.h"
#include "tfidf.h"
#include "spatial_verif.h"
#include "document_map.h"
#include "evaluator.h"
#include "exclude_docs.h"
#include "image_graph.h"
#include "tfidf_aug.h"


int main(int argc, char* argv[]) {
    
    MPI_INIT_ENV;
    
    parQueue<evaluator::APresultType> evalParQueue_obj(false);
    
    // process input arguments (stupid, make better)
    
    std::string prefix= "oxc1_5k";
    if (argc>1)
        prefix= argv[1];
    bool useHellinger= false;
    if (argc>2 && argv[2][0]=='h')
        useHellinger= true;
    
    bool useParis= false;
    std::string dParis= "";
    if (argc>3 && argv[3][0]=='p'){
        useParis= true;
        dParis= "_dParis";
    }
    
    
    if (evalParQueue_obj.getRank()==0){
        std::cout<<"prefix= "<< prefix <<"\n";
        std::cout<<"hell= "<< useHellinger <<"\n";
        std::cout<<"useParis= "<< useParis<<"\n";
        std::cout<<"\n";
    }
    
    const char datapath[]= "/home/relja/Relja/Data";
    char dset_fn[200], iidx_fn[200], fidx_fn[200], wght_fn[200];
    
    uint32_t k= 1000000;
    uint32_t seed= 43;
    
    std::string detdesc=(useHellinger?"hesaff_sift_hell":"hesaff_sift");
    
    sprintf(dset_fn, "%s/%s/dset_%s.db", datapath, prefix.c_str(), prefix.c_str());
    
    std::string dirname= ( (useParis || useHellinger || prefix!="oxc1_5k")?"ox_exp":prefix);
    
    sprintf(iidx_fn, "%s/%s/iidx_%s_%s%s_%d_%d.bin", datapath, dirname.c_str(), prefix.c_str(), detdesc.c_str(), dParis.c_str(), k, seed);
    sprintf(fidx_fn, "%s/%s/word_%s_%s%s_%d_%d.h5", datapath, dirname.c_str(), prefix.c_str(), detdesc.c_str(), dParis.c_str(), k, seed);
    sprintf(wght_fn, "%s/%s/wght_%s_%s%s_%d_%d.rr.bin", datapath, dirname.c_str(), prefix.c_str(), detdesc.c_str(), dParis.c_str(), k, seed);
    
//     documentMap docMap( dset_fn, prefix.c_str() );
    documentMap docMap( (std::string("/home/relja/Relja/Data/")+prefix+"/docMap_"+prefix+".bin").c_str() );
    iidxWrapperJpGen<uint32_t> *iidx_obj= new iidxWrapperJpGen<uint32_t>( iidx_fn );
    forwardIndex *fidx_obj;
    if (useParis && useHellinger)
        fidx_obj= new fidxWrapperJpDb5_FixK( fidx_fn, docMap, 0 );
    else
        fidx_obj= new fidxWrapperJpDb5_HardJP( fidx_fn, docMap );
    
    // wrap around tfidf to exclude gt query images from results
    std::string gt_fn=
        (boost::format("/home/relja/Relja/Code/relja_retrieval/temp/oxford_%s.gt.bin")
        % prefix ).str();
    evaluator eval_obj( gt_fn.c_str(), false );
    
    
    //------- image graph
    
    std::string graph_fn=
        (boost::format("%s/misc/graph_%s_%s%s.bin")
        % datapath % prefix % detdesc % dParis ).str();
    
    imageGraph *imageGraph_obj= NULL;
    
    if ( boost::filesystem::exists( graph_fn.c_str() ) ){
        std::set<uint32_t> excludes;
        excludeDocs::computeExcludes(eval_obj, excludes);
        imageGraph_obj= new imageGraph( graph_fn.c_str(), &(excludes) );
    } else {
        // compute image graph
        
        tfidf *tfidf_obj= NULL;
        
        if ( boost::filesystem::exists( wght_fn ) ){
            tfidf_obj= new tfidf( iidx_obj, fidx_obj, wght_fn );
        } else {
            tfidf_obj= new tfidf( iidx_obj, fidx_obj );
            tfidf_obj->saveToFile( wght_fn );
        }
        
        excludeDocs *tfidf_excl_obj= new excludeDocs( *tfidf_obj, eval_obj );
        
        spatialVerif spatVer_obj( tfidf_excl_obj, fidx_obj, NULL, false );
        
        imageGraph_obj= new imageGraph( graph_fn.c_str(), iidx_obj->numDocs(), spatVer_obj, 200, 7.0, &(tfidf_excl_obj->excludes) );
        
        delete tfidf_obj;
        delete tfidf_excl_obj;
    }
    
    imageGraph_obj->keepVerified( 50, 15.0, true, 3 );
    
    
    //------- augmented tfidf
    
    std::string tfidfaug_fn=
        (boost::format("%s/misc/tfidfaug_l2_idf_%s_%s%s.bin")
        % datapath % prefix % detdesc % dParis ).str();
    
    tfidfAug *tfidfAug_obj= NULL;
    if ( boost::filesystem::exists( tfidfaug_fn.c_str() ) ){
        tfidfAug_obj= new tfidfAug( iidx_obj, fidx_obj, *imageGraph_obj, tfidfaug_fn.c_str() );
    } else {
        tfidfAug_obj= new tfidfAug( iidx_obj, fidx_obj, *imageGraph_obj );
        tfidfAug_obj->saveToFile( tfidfaug_fn.c_str() );
    }
    
    
    //------- evaluate
    
    std::vector<double> APs;
    
    // tfidfAug
    double mAP_tfidfaug= eval_obj.computeMAP( *tfidfAug_obj, APs, true );
    
    // spatial verification (but no spatial aug)
    spatialVerif spatVer_obj2( tfidfAug_obj, fidx_obj, NULL, false );
    double mAP_tfidfaug_sp= eval_obj.computeMAP( spatVer_obj2, APs, true );
    
    printf("mAP_tfidfaug    = %.4f\n", mAP_tfidfaug);
    printf("mAP_tfidfaug_sp = %.4f\n", mAP_tfidfaug_sp);
    
    //------- cleanup
    
    delete iidx_obj;
    delete fidx_obj;
    delete imageGraph_obj;
    delete tfidfAug_obj;
    
}
