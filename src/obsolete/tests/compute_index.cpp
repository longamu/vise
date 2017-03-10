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
#include "iidx_wrapper_jp_gen.h"
#include "tfidf.h"
#include "document_map.h"
#include "evaluator.h"
#include "soft_assigner.h"
#include "desc_wrapper_jp_db5.h"
#include "desc_getter_from_file.h"
#include "desc_from_file_to_hell.h"
#include "clst_centres.h"



int main(int argc, char* argv[]) {
        
    MPI_INIT_ENV;
    
    std::string prefix, docMapFn, gtFn, featFn, clstFn, asgnFn, fidxFn, iidxFn;
    
    bool doOx5k= !(argc>1 && argv[1][0]!='5');
    bool useHell= (argc>2 && argv[2][0]=='h');
    bool clstOx= !(argc>3 && argv[3][0]=='p');
    
    {
        std::cout<<"doOx5k= "<< doOx5k <<"\n";
        std::cout<<"hell= "<< useHell <<"\n";
        std::cout<<"clstOx= "<< clstOx<<"\n";
        std::cout<<"\n";
    }
    
    const char newExp[]= "_softmax";
    
    if (doOx5k){
        prefix= "oxc1_5k";
        docMapFn= "/tmp/relja/Relja/Data/ox_exp/dset_oxc1_5k.db";
        gtFn= "/tmp/relja/Relja/Code/relja_retrieval/temp/oxford_oxc1_5k.gt.bin";
        featFn= "/tmp/relja/ox_exp/feat_oxc1_5k_hesaff_sift_prague.h5";
    } else {
        prefix= "ox100k";
        docMapFn= "/tmp/relja/Relja/Data/ox_exp/dset_ox100k.db";
        gtFn= "/tmp/relja/Relja/Code/relja_retrieval/temp/oxford_ox100k.gt.bin";
        featFn= "/tmp/relja/ox_exp/feat_ox100k_hesaff_sift_prague.h5";
    }
        
    
    if (!useHell) {
        
        // SIFT
        if (doOx5k){
            if (clstOx){
                asgnFn= "/tmp/relja/ox_exp/soft_asgn_oxc1_5k_hesaff_sift_prague_1000000_43.h5";
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_oxc1_5k_hesaff_sift_prague%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/ox_exp/iidx_oxc1_5k_hesaff_sift_prague%s_1000000_43.bin") % newExp ).str();
            } else {
                asgnFn= "/tmp/relja/ox_exp/soft_asgn_oxc1_5k_hesaff_sift_prague_dParis_1000000_43.h5";
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_oxc1_5k_hesaff_sift_prague_dParis%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/ox_exp/iidx_oxc1_5k_hesaff_sift_prague_dParis%s_1000000_43.bin") % newExp ).str();
            }
        } else {
            if (clstOx){
                asgnFn= "/tmp/relja/ox_exp/soft_asgn_ox100k_hesaff_sift_prague_1000000_43.h5";
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_ox100k_hesaff_sift_prague%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/ox_exp/iidx_ox100k_hesaff_sift_prague%s_1000000_43.bin") % newExp ).str();
            } else {
                asgnFn= "/tmp/relja/ox_exp/soft_asgn_ox100k_hesaff_sift_prague_dParis_1000000_43.h5";
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_ox100k_hesaff_sift_prague_dParis%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/ox_exp/iidx_ox100k_hesaff_sift_prague_dParis%s_1000000_43.bin") % newExp ).str();
            }
        }
        
        if (clstOx){
            clstFn= "/tmp/relja/Relja/Data/ox_exp/clst_oxc1_5k_hesaff_sift_prague_1000000_43.bin";
        } else {
            clstFn= "/tmp/relja/Relja/Data/ox_exp/clst_paris_hesaff_sift_prague_1000000_43.bin";
        }
        
    } else {
        
        // Hellinger
        if (doOx5k){
            if (clstOx){
                asgnFn= "/tmp/relja/ox_exp/soft_asgn_oxc1_5k_hesaff_sift_prague_hell_1000000_43.h5";
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_oxc1_5k_hesaff_sift_prague_hell%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/ox_exp/iidx_oxc1_5k_hesaff_sift_prague_hell%s_1000000_43.bin") % newExp ).str();
            } else {
                asgnFn= "/tmp/relja/ox_exp/soft_asgn_oxc1_5k_hesaff_sift_prague_hell_dParis_1000000_43.h5";
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_oxc1_5k_hesaff_sift_prague_hell_dParis%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/ox_exp/iidx_oxc1_5k_hesaff_sift_prague_hell_dParis%s_1000000_43.bin") % newExp ).str();
            }
        } else {
            if (clstOx){
                asgnFn= "/tmp/relja/ox_exp/soft_asgn_ox100k_hesaff_sift_prague_hell_1000000_43.h5";
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_ox100k_hesaff_sift_prague_hell%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/ox_exp/iidx_ox100k_hesaff_sift_prague_hell%s_1000000_43.bin") % newExp ).str();
            } else {
                asgnFn= "/tmp/relja/ox_exp/soft_asgn_ox100k_hesaff_sift_prague_hell_dParis_1000000_43.h5";
                fidxFn= (boost::format( "/tmp/relja/ox_exp/word_ox100k_hesaff_sift_prague_hell_dParis%s_1000000_43.h5") % newExp ).str();
                iidxFn= (boost::format( "/tmp/relja/ox_exp/iidx_ox100k_hesaff_sift_prague_hell_dParis%s_1000000_43.bin") % newExp ).str();
            }
        }
        
        if (clstOx){
            clstFn= "/tmp/relja/Relja/Data/ox_exp/clst_oxc1_5k_hesaff_sift_prague_hell_1000000_43.bin";
        } else {
            clstFn= "/tmp/relja/Relja/Data/ox_exp/clst_paris_hesaff_sift_prague_hell_1000000_43.bin";
        }
        
    }
    
    documentMap docMap( docMapFn.c_str(), prefix.c_str() );
    
    const uint KNN= 3;
    float sigmaSq;
    
    if (!useHell)
        sigmaSq= 6250;
    else
        sigmaSq= 0.02;
    
    descGetterFromFile *desc_obj= NULL, *desc_obj_temp= NULL;
    
    #if 1
    SA_exp SA_obj( sigmaSq );
    desc_obj= NULL; desc_obj_temp= NULL;
    clstCentres *clstC= NULL;
    #else
    clstCentres *clstC= new clstCentres( clstFn.c_str() );
    SA_LLC SA_obj( clstC, sigmaSq, 0.1 );
<<<<<<< HEAD
    desc_obj_temp= new descWrapperJpDb5<uint8_t>(featFn.c_str(),128);
    if (!useHell)
        desc_obj= desc_obj_temp;
    else
        desc_obj= new descToHell( desc_obj_temp );
=======
    feat_obj_temp= new featWrapperJpDb5<uint8_t>(featFn.c_str(),128);
//     if (!useHell)
//         feat_obj= feat_obj_temp;
//     else
        feat_obj= new featToHell( feat_obj_temp );
>>>>>>> master
    #endif
    
    fidxWrapperJpDb5_FixK::computeWords( docMap, fidxFn.c_str(), desc_obj, featFn.c_str(), asgnFn.c_str(), KNN, &SA_obj );
    
    if (desc_obj_temp!=NULL){
        delete desc_obj;
        if (useHell)
            delete desc_obj_temp;
    }
    if (clstC!=NULL)
        delete clstC;
    
    fidxWrapperJpDb5_FixK fidx_obj(fidxFn.c_str(), docMap, KNN);
    
    
    #if 0
    if (true){
        // compute iidx
        iidxWrapperJpGen<float> iidx_obj( iidxFn.c_str(), fidx_obj, "/data4/relja/Data/tmp" );
    }
    iidxWrapperJpGen<float> iidx_obj( iidxFn.c_str() );
    
    char fn[] = "/home/relja/Relja/Code/relja_retrieval/temp/tfidf.ser.bin";
    tfidf tfidf_obj( &iidx_obj, &fidx_obj ); tfidf_obj.saveToFile( fn );
    
    std::vector<indScorePair> queryRes;
    
    const char hash[]= "60fd73963102f86baf08325631f8912db34acba7fb46cc9a41b818099276187e";
    uint32_t docID= docMap.h2i( hash );
    
    queryIn queryIn_obj(docID, true, 136.5, 648.5, 34.1, 955.7 );
    tfidf_obj.internalQuery( queryIn_obj, queryRes );
    
    std::cout<< "query: "<< hash <<"\n\n";
    
    for (int i=0; i<5; ++i){
        std::cout<< i <<" "<< queryRes[i].second <<" "<< docMap.i2h( queryRes[i].first ) <<"\n";
    }
    
    evaluator eval_obj( gtFn.c_str(), false );
    std::vector<double> APs;
    double mAP_tfidf= eval_obj.computeMAP( tfidf_obj, APs, false );
    printf("mAP_tfidf= %.4f\n", mAP_tfidf);
    #endif
    
    return 0;
    
}
