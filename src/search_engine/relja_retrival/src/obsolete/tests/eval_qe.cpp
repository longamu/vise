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

#include "par_queue.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "fidx_wrapper_jp_db5.h"
#include "iidx_wrapper_jp_gen.h"
#include "tfidf.h"
#include "spatial_verif.h"
#include "query_expand.h"
#include "query_expand_svm.h"
#include "document_map.h"
#include "evaluator.h"

#include "iidx_in_ram.h"


int main(int argc, char* argv[]) {
    
    MPI_INIT_ENV;
    
    parQueue<evaluator::APresultType> evalParQueue_obj(true, 0);
    
    // process input arguments (stupid, make better)
    
    std::string prefix= "oxc1_5k";
    if (argc>1)
        prefix= argv[1];
    
    bool useHellinger= false;
    if (argc>2 && argv[2][0]=='h')
        useHellinger= true;
    
    bool usePrague= (argc>2 && strlen(argv[2])>1 && argv[2][1]=='p');
    
    bool useParis= false;
    std::string dParis= "";
    if (argc>3 && argv[3][0]=='p'){
        useParis= true;
        dParis= "_dParis";
    }
    
    bool qSA= (argc>3 && strlen(argv[3])>1 && argv[3][1]=='q');
    
    bool qeSVM= !(argc>4 && argv[4][0]=='m');
    
    bool aug= (argc>5 && argv[5][0]=='a');
    
    
    if (evalParQueue_obj.getRank()==0){
        std::cout<<"prefix= "<< prefix <<"\n";
        std::cout<<"hell= "<< useHellinger <<"\n";
        std::cout<<"usePrague= "<< usePrague <<"\n";
        std::cout<<"useParis= "<< useParis<<"\n";
        std::cout<<"qSA= "<< qSA<<"\n";
        std::cout<<"qeSVM= "<< qeSVM<<"\n";
        std::cout<<"aug= "<< aug <<"\n";
        std::cout<<"\n";
    }
    
    char dset_fn[200], iidx_fn[200], fidxQSA_fn[200], fidxDB_fn[200], wght_fn[200];
    
    uint32_t k= 1000000;
    uint32_t seed= 43;
    
    std::string detdesc=std::string( "hesaff_sift" ) + (usePrague?"_prague":"") + (useHellinger?"_hell":"");
    
    std::string datapath= std::string(getenv("HOME"))+"/Relja/Data";
    
    sprintf(dset_fn, "%s/ox_exp/dset_%s.db", datapath.c_str(), prefix.c_str());
    
    sprintf(iidx_fn, "%s/ox_exp/iidx%s_%s_%s%s_%d_%d.bin",  datapath.c_str(), (aug?"aug":""), prefix.c_str(), detdesc.c_str(), dParis.c_str(), k, seed);
    sprintf(fidxQSA_fn, "%s/ox_exp/word_oxc1_5k_%s%s_softmax_%d_%d.h5", datapath.c_str(), detdesc.c_str(), dParis.c_str(), k, seed);
    sprintf(fidxDB_fn, "%s/ox_exp/word_%s_%s%s_%d_%d.h5", datapath.c_str(), prefix.c_str(), detdesc.c_str(), dParis.c_str(), k, seed);
    if (!aug)
        sprintf(wght_fn, "%s/ox_exp/wght_%s_%s%s_%d_%d.rr.bin", datapath.c_str(), prefix.c_str(), detdesc.c_str(), dParis.c_str(), k, seed);
    else
        sprintf(wght_fn, "/tmp/relja/Relja/Code/relja_retrieval/temp/tfidf_aug_%s_%s%s_1000000_43.bin",prefix.c_str(),detdesc.c_str(), (useParis?"_dParis":""));
    
//     documentMap docMap( dset_fn, prefix.c_str() );
    documentMap docMap( (datapath+"/"+prefix+"/docMap_"+prefix+".bin").c_str() );
    
    std::cout<<iidx_fn<<"\n";
    
    invertedIndex *iidx_obj;
    if (!aug) {
        #if 0
        iidx_obj= new iidxWrapperJpGen<uint32_t>( iidx_fn );
        #else
        iidx_obj= new iidxInRam( iidx_fn );
        #endif
    }
    else {
        #if 0
        iidx_obj= new iidxWrapperJpGen<float>( iidx_fn );
        #else
        iidxWrapperJpGen<float> temp( iidx_fn );
        iidx_obj= new iidxInRam( temp );
        #endif
    }
    
    std::cout<<fidxDB_fn<<"\n";
    
    forwardIndex *fidxDB_obj;
    if (useParis && useHellinger && !usePrague)
        fidxDB_obj= new fidxWrapperJpDb5_FixK( fidxDB_fn, docMap, 0 );
    else
        fidxDB_obj= new fidxWrapperJpDb5_HardJP( fidxDB_fn, docMap );
    
    forwardIndex *fidxQ_obj;
    if (qSA){
        std::cout<<fidxQSA_fn<<"\n";
        fidxQ_obj= new fidxWrapperJpDb5_FixK(fidxQSA_fn, docMap, 3);
    } else
        fidxQ_obj= fidxDB_obj;
    
    //------- tfidf
    
    tfidf *tfidf_obj= NULL;
    
    std::cout<<wght_fn<<"\n";
    
    if ( !boost::filesystem::exists( wght_fn ) ){
        tfidf tfidf_obj( iidx_obj, fidxDB_obj ); tfidf_obj.saveToFile( wght_fn );
    }
    
    tfidf_obj= new tfidf( iidx_obj, fidxQ_obj, wght_fn );
    
    //------- spatial verification / query expansion
    
    spatParams spatParams_obj( (usePrague?1000:200) );
    spatialVerif spatVer_obj( tfidf_obj, fidxDB_obj, &spatParams_obj, false );
    
    queryExpand *queryExp_obj;
    if (qeSVM)
        queryExp_obj= new queryExpandSVM( iidx_obj, tfidf_obj, spatVer_obj );
    else
        queryExp_obj= new queryExpand( spatVer_obj );
    
    //------- evaluate
    
    std::string gt_fn=
        (boost::format("/home/relja/Relja/Code/relja_retrieval/temp/oxford_%s.gt.bin")
        % prefix ).str();
    evaluator eval_obj( gt_fn.c_str(), false );
    
    std::vector<double> APs;
    
    // tfidf
    double mAP_tfidf= eval_obj.computeMAP( *tfidf_obj, APs, true, true, &evalParQueue_obj );
        
    // spatial verification
    double mAP_sp= eval_obj.computeMAP( spatVer_obj, APs, true, true, &evalParQueue_obj );
    
    // query expansion
    double mAP_qe= eval_obj.computeMAP( *queryExp_obj, APs, true, true, &evalParQueue_obj );
    
    if (evalParQueue_obj.getRank()==0){
        printf("mAP_tfidf= %.4f\n", mAP_tfidf);
        printf("mAP_sp   = %.4f\n", mAP_sp);
        printf("mAP_qe   = %.4f\n", mAP_qe);
    }
    
    //------- cleanup
    
    delete iidx_obj;
    delete fidxDB_obj;
    if (qSA)
        delete fidxQ_obj;
    delete tfidf_obj;
    delete queryExp_obj;
    
}
