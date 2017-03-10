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

#include "util.h"
#include "par_queue.h"
#include <boost/format.hpp>

#include "fidx_wrapper_jp_db5.h"
#include "fidx_aug.h"
#include "iidx_wrapper_jp_gen.h"
#include "tfidf.h"
#include "spatial_verif.h"
#include "document_map.h"
#include "evaluator.h"
#include "exclude_docs.h"



int main(int argc, char* argv[]) {
        
    MPI_INIT_ENV;
    
    std::string prefix, docMapFn, gtFn, fidxFn, iidxFn, graphFn;
    
    bool doOx5k= !(argc>1 && argv[1][0]!='5');
    bool useHell= (argc>2 && argv[2][0]=='h');
    bool usePrague= (argc>2 && strlen(argv[2])>1 && argv[2][1]=='p');
    bool clstOx= !(argc>3 && argv[3][0]=='p');
    
    {
        std::cout<<"doOx5k= "<< doOx5k <<"\n";
        std::cout<<"hell= "<< useHell <<"\n";
        std::cout<<"usePrague= "<< usePrague <<"\n";
        std::cout<<"clstOx= "<< clstOx<<"\n";
        std::cout<<"\n";
    }
    
    const char newExp[]= "";
    
    if (doOx5k){
        prefix= "oxc1_5k";
        docMapFn= "/tmp/relja/Relja/Data/oxc1_5k/dset_oxc1_5k.db";
        gtFn= "/tmp/relja/Relja/Code/relja_retrieval/temp/oxford_oxc1_5k.gt.bin";
    } else {
        prefix= "ox100k";
        docMapFn= "/tmp/relja/Relja/Data/ox100k/dset_ox100k.db";
        gtFn= "/tmp/relja/Relja/Code/relja_retrieval/temp/oxford_ox100k.gt.bin";
    }
    
    
    std::string detdesc=std::string( "hesaff_sift" ) + (usePrague?"_prague":"") + (useHell?"_hell":"");
    
    if (!useHell) {
        
        // SIFT
        if (doOx5k){
            if (clstOx){
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_oxc1_5k_hesaff_sift%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/Relja/Data/ox_exp/iidxaug_oxc1_5k_hesaff_sift%s_1000000_43.bin") % newExp ).str();
                graphFn= (boost::format("/tmp/relja/Relja/Data/misc/graph_%s_hesaff_sift.bin") % prefix ).str();
            } else {
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_oxc1_5k_hesaff_sift_dParis%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/Relja/Data/ox_exp/iidxaug_oxc1_5k_hesaff_sift_dParis%s_1000000_43.bin") % newExp ).str();
                graphFn= (boost::format("/tmp/relja/Relja/Data/misc/graph_%s_hesaff_sift_dParis.bin") % prefix ).str();
            }
        } else {
            if (clstOx){
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_ox100k_hesaff_sift%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/Relja/Data/ox_exp/iidxaug_ox100k_hesaff_sift%s_1000000_43.bin") % newExp ).str();
                graphFn= (boost::format("/tmp/relja/Relja/Data/misc/graph_%s_hesaff_sift.bin") % prefix ).str();
            } else {
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_ox100k_hesaff_sift_dParis%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/Relja/Data/ox_exp/iidxaug_ox100k_hesaff_sift_dParis%s_1000000_43.bin") % newExp ).str();
                graphFn= (boost::format("/tmp/relja/Relja/Data/misc/graph_%s_hesaff_sift_dParis.bin") % prefix ).str();
            }
        }
        
    } else {
        
        // Hellinger
        if (doOx5k){
            if (clstOx){
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_oxc1_5k_hesaff_sift_hell%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/Relja/Data/ox_exp/iidxaug_oxc1_5k_hesaff_sift_hell%s_1000000_43.bin") % newExp ).str();
                graphFn= (boost::format("/tmp/relja/Relja/Data/misc/graph_%s_hesaff_sift_hell.bin") % prefix ).str();
            } else {
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_oxc1_5k_hesaff_sift_hell_dParis%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/Relja/Data/ox_exp/iidxaug_oxc1_5k_hesaff_sift_hell_dParis%s_1000000_43.bin") % newExp ).str();
                graphFn= (boost::format("/tmp/relja/Relja/Data/misc/graph_%s_hesaff_sift_hell_dParis.bin") % prefix ).str();
            }
        } else {
            if (clstOx){
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_ox100k_hesaff_sift_hell%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/Relja/Data/ox_exp/iidxaug_ox100k_hesaff_sift_hell%s_1000000_43.bin") % newExp ).str();
                graphFn= (boost::format("/tmp/relja/Relja/Data/misc/graph_%s_hesaff_sift_hell.bin") % prefix ).str();
            } else {
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_ox100k_hesaff_sift_hell_dParis%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/Relja/Data/ox_exp/iidxaug_ox100k_hesaff_sift_hell_dParis%s_1000000_43.bin") % newExp ).str();
                graphFn= (boost::format("/tmp/relja/Relja/Data/misc/graph_%s_hesaff_sift_hell_dParis.bin") % prefix ).str();
            }
        }
        
    }
    
    documentMap docMap( docMapFn.c_str(), prefix.c_str() );
    evaluator eval_obj( gtFn.c_str(), false );
    
    forwardIndex *fidx_obj;
    if (!clstOx && useHell && !usePrague)
        fidx_obj= new fidxWrapperJpDb5_FixK( fidxFn.c_str(), docMap, 0 );
    else
        fidx_obj= new fidxWrapperJpDb5_HardJP( fidxFn.c_str(), docMap );
    
    #if 1
    if (true){
        
        std::set<uint32_t> excludes;
        excludeDocs::computeExcludes(eval_obj, excludes);
        imageGraph imageGraph_obj( graphFn.c_str(), &(excludes) );
        imageGraph_obj.keepVerified( 50, 15.0, true, 3 );
        fidxAug fidxAug_obj( fidx_obj, &imageGraph_obj );
        
        // compute iidx
        iidxWrapperJpGen<float> iidx_obj( iidxFn.c_str(), fidxAug_obj,
#ifdef ISTITAN
            "/tmp"
#else
            "/data4/relja/Data/tmp"
#endif
        );
        
    }
    iidxWrapperJpGen<float> iidx_obj( iidxFn.c_str() );
    
    char wght_fn[200];
    sprintf(wght_fn, "/tmp/relja/Relja/Code/relja_retrieval/temp/tfidf_aug_%s_%s%s_1000000_43.bin",prefix.c_str(),detdesc.c_str(), (clstOx?"":"_dParis") );
    tfidf tfidf_obj( &iidx_obj, fidx_obj ); tfidf_obj.saveToFile( wght_fn );
    
    spatParams spatParams_obj( (usePrague?1000:200) );
    spatialVerif spatVer_obj( &tfidf_obj, fidx_obj, &spatParams_obj, false );
    
    std::vector<double> APs;
    double mAP_tfidf= eval_obj.computeMAP( tfidf_obj, APs, false, true );
    double mAP_sp= eval_obj.computeMAP( spatVer_obj, APs, false, true );
    printf("mAP_tfidf= %.4f\n", mAP_tfidf);
    printf("mAP_sp   = %.4f\n", mAP_sp);
    #endif
    
    delete fidx_obj;
    
    return 0;
    
}
