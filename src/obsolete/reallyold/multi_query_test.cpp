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
#include "tfidf.h"
#include "spatial_verif.h"
#include "document_map.h"
#include "evaluator.h"
#include "multi_query.h"
#include "multi_query_svm.h"




// this is only a fake class to test multiQuery on Oxford buildings, note the "static" variable which messes with parallel evaluation of this thing
static uint32_t queryID= 0;
static std::vector<indScorePair> prevQueryRes;

class multiQueryRetFake : public bowRetriever {
    
    public:
        
        multiQueryRetFake( multiQuery *aMultiQuery_obj, evaluator *aEvaluator_obj, forwardIndex *aForwardIndex_obj ) : bowRetriever(aForwardIndex_obj), multiQuery_obj(aMultiQuery_obj), evaluator_obj(aEvaluator_obj), forwardIndex_obj(aForwardIndex_obj) {
        }
        
        void
            query( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const {
                
                if (queryID%5!=0){
                    queryRes= prevQueryRes;
                    ++queryID;
                    return;
                }
                if (queryID>=55){
                    // new query
                    queryID= 0;
                }
                
                multiQuery::retsType represent_objs;
                uint32_t qID= floor(queryID/5)*5;
                for (uint32_t i=0; i<5; ++i){
                    represent_objs.push_back( new represent(*forwardIndex_obj, evaluator_obj->queries[qID+i]) );
                }
                
                multiQuery_obj->query( represent_objs, queryRes );
                prevQueryRes= queryRes;
                
                for (uint32_t i=0; i<5; ++i){
                    delete represent_objs[i];
                }
                
                ++queryID;
                
            }
    
    private:
        
        std::vector< represent* > repIn;
        
        multiQuery *multiQuery_obj;
        evaluator *evaluator_obj;
        forwardIndex *forwardIndex_obj;
    
};



class multiQueryFromWords : public bowRetriever {
    
    public:
        
        multiQueryFromWords( multiQuery *aMultiQuery_obj, forwardIndex *aForwardIndexDB_obj, uint32_t aNumTop= 8, bool aSameRes= true, bool aUsePrague= true ) : bowRetriever(aForwardIndexDB_obj), multiQuery_obj(aMultiQuery_obj), numTop(aNumTop), sameRes(aSameRes), usePrague(aUsePrague) {
        }
        
        void
            query( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const {
                
                if (sameRes && queryID%5!=0){
                    queryRes= prevQueryRes;
                    ++queryID;
                    return;
                }
                
                if (queryID>=55){
                    // new query
                    queryID= 0;
                }
                
                char fidxQ_fn[]="/home/relja/Relja/Code/misc/multiquery/data/wordsStrict/word_oxq00_hesaff_prague_rootsift_1000000_43.h5xxxxxxxxx";
                char dset_fn[]="/home/relja/Relja/Code/misc/multiquery/data/wordsStrict/dset_00.db";
                sprintf(fidxQ_fn, "/home/relja/Relja/Code/misc/multiquery/data/wordsStrict/word_oxq%.2d_hesaff_%srootsift_1000000_43.h5", static_cast<int>(floor(queryID/5)), usePrague?"prague_":"" );
                sprintf(dset_fn, "/home/relja/Relja/Code/misc/multiquery/data/wordsStrict/dset_%.2d.db", static_cast<int>(floor(queryID/5)) );
                
                documentMap docMap( dset_fn, "queries" );
                
                fidxWrapperJpDb5_HardJP fidxQ( fidxQ_fn, docMap );
                
                multiQuery::retsType represent_objs;
                for (uint32_t i=0; i<numTop; ++i){
                    query qIn( i, false );
                    represent_objs.push_back( new represent(fidxQ, qIn) );
                }
                
                multiQuery_obj->query( represent_objs, queryRes );
                prevQueryRes= queryRes;
                
                for (uint32_t i=0; i<numTop; ++i){
                    delete represent_objs[i];
                }
                
                ++queryID;
                
            }
    
    private:
        
        std::vector< represent* > repIn;
        
        multiQuery *multiQuery_obj;
        
        const uint32_t numTop;
        const bool sameRes, usePrague;
    
};



class singleRetToMQ : public multiQuery {
    
    public:
        
        singleRetToMQ( bowRetriever const &aRetriever_obj, uint32_t aIndex= 0 ) : retriever_obj(&aRetriever_obj), index(aIndex) { }
        
        void
            query( retsType &represent_objs, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const {
                
                retriever_obj->query( *(represent_objs[index]), queryRes, toReturn );
                
            }
    
    private:
        
        bowRetriever const *retriever_obj;
        uint32_t const index;
        
};



double
bestSingle( std::vector<double> const &APs ){
    
    double res= 0.0;
    
    for (uint32_t i=0; i<11; ++i){
        double best= APs[i*5];
        for (uint32_t j=1; j<5; ++j){
            double c= APs[i*5+j];
            best= (best>c?best:c);
        }
        res+= best;
    }
    
    return res/11;
    
}





int main(int argc, char* argv[]) {
    
    MPI_INIT_ENV;
    
    std::string datapath= std::string(getenv("HOME"))+"/Relja/Data";
    
    std::string prefix= "oxc1_5k";
    bool usePrague= false;
    bool googleQueries= true;
    
    if (argc>1){
        
        prefix= (argv[1][0]=='5'?"oxc1_5k":"ox100k");
        usePrague= ( strlen(argv[1])>1 && argv[1][1]=='p' );
        googleQueries= !( strlen(argv[1])>2 && argv[1][2]=='o');
        
    }
    
    std::cout<< "Dataset: "<<prefix<<"\n";
    std::cout<< "Prague detector: "<<usePrague<<"\n";
    std::cout<< "Google queries: "<<googleQueries<<"\n";
    
    uint32_t k= 1000000;
    uint32_t seed= 43;
    std::string detdesc= (boost::format("hesaff_sift_%shell") % (usePrague?"prague_":"")).str();
    uint32_t numTop= 8;
    
    std::string dsetFn= ( boost::format("%s/%s/dset_%s.db") % datapath % prefix % prefix ).str();
    std::string iidxFn= ( boost::format("%s/ox_exp/iidx_%s_%s_%d_%d.bin") % datapath % prefix % detdesc % k % seed ).str();
    std::string fidxFn= ( boost::format("%s/ox_exp/word_%s_%s_%d_%d.h5") % datapath % prefix % detdesc % k % seed ).str();
    std::string wghtFn= ( boost::format("%s/ox_exp/wght_%s_%s_%d_%d.rr.bin") % datapath % prefix % detdesc % k % seed ).str();
    
    std::cout<<dsetFn.c_str()<<"\n";
    documentMap docMap( dsetFn.c_str(), prefix.c_str() );
    std::cout<<dsetFn.c_str()<<"\n";
    //iidxWrapperJpGen<uint32_t> *iidx_obj= new iidxWrapperJpGen<uint32_t>( iidxFn.c_str() );
    invertedIndex *iidx_obj= new iidxInRam( iidxFn.c_str() );
    fidxWrapperJpDb5_HardJP *fidx_obj= new fidxWrapperJpDb5_HardJP( fidxFn.c_str(), docMap);
    tfidf *tfidf_obj= NULL;
    
    if ( boost::filesystem::exists( wghtFn ) ){
        tfidf_obj= new tfidf( iidx_obj, fidx_obj, wghtFn.c_str() );
    } else {
        tfidf_obj= new tfidf( iidx_obj, fidx_obj );
        tfidf_obj->saveToFile( wghtFn.c_str() );
    }
    
    spatialVerif spatVer_obj( tfidf_obj, fidx_obj, fidx_obj, NULL, false, false );
    
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
    
    parQueue<evaluator::APresultType> evalParQueue_obj(true,1); // important that only 1 thread due to static stuff
    
    bool noSR= (argc>1 && strlen(argv[1])>3 && argv[1][3]!='s');
    std::cout<< (noSR?"noSR":"SR")<<" ";
    
    
    if (argc>2 && argv[2][0]!='b'){
        
        
        multiQuery *mq= NULL, *mqFirst= NULL;
        exemplarSVM *ESVM_obj= NULL;
        
        
        if (argv[2][0]=='j'){
            
            std::cout<<"Joint-";
            
            if (argc>3 && argv[3][0]!='a'){
                
                std::cout<<"SVM";
                
                mq= new multiQueryJointSVM( *tfidf_obj, *fidx_obj );
                
            } else {
                
                std::cout<<"Avg";
                
                mq= new multiQueryJointAvg( *tfidf_obj );
                
            }
            
            if (!noSR){
                mqFirst= mq;
                mq= new multiQueryPostSpatial( *mqFirst, spatVer_obj );
            }
            
        } else {
            
            std::cout<<"MQ-";
            
            if (argc>3 && argv[3][0]!='e' && argv[3][0]!='c'){
                
                if (argv[3][0]=='m'){
                    
                    std::cout<<"Max";
                    
                    if (noSR)
                        mq= new multiQueryMax( *tfidf_obj );
                    else
                        mq= new multiQueryMax( spatVer_obj );
                    
                } else {
                    
                    std::cout<<"Avg";
                    
                    if (noSR)
                        mq= new multiQuerySum( *tfidf_obj );
                    else
                        mq= new multiQuerySum( spatVer_obj );
                    
                }
                
            } else {
                
                std::cout<<"ESVM";
                
                if (argv[3][0]=='c'){
                    
                    std::cout<<"c";
                    
                    if (!noSR)
                        ESVM_obj= new exemplarSVM(*tfidf_obj, fidx_obj, &spatVer_obj, true);
                    
                } else {
                    
                    std::cout<<"e";
                    
                    if (noSR)
                        ESVM_obj= new exemplarSVM(*tfidf_obj, fidx_obj, NULL, false);
                    else
                        ESVM_obj= new exemplarSVM(*tfidf_obj, fidx_obj, &spatVer_obj, false);
                    
                }
                
                mq= new multiQueryMax( *ESVM_obj );
                
            }
            
        }
        
        bowRetriever *mqRetriever;
        if (googleQueries)
            mqRetriever= new multiQueryFromWords( mq, fidx_obj, numTop, true, usePrague );
        else
            mqRetriever= new multiQueryRetFake( mq, eval_obj, fidx_obj );
        
        std::cout<<"\n";
        double mAP= eval_obj->computeMAP( *mqRetriever, APs, false, true, &evalParQueue_obj );
        printf(" %.4f\n", mAP);
        
        delete mqRetriever;
        delete mq;
        if (mqFirst!=NULL)
            delete mqFirst;
        if (ESVM_obj!=NULL)
            delete ESVM_obj;
        
    } else {
        
        bowRetriever *ret;
        
        if (noSR)
            ret= tfidf_obj;
        else
            ret= &spatVer_obj;
        
        singleRetToMQ mq(*ret, 0);
        retriever *mqRetriever;
        if (googleQueries)
            mqRetriever= new multiQueryFromWords( &mq, fidx_obj, 1, true, usePrague );
        else
            mqRetriever= ret;
        
        std::cout<<"\n";
        double mAP= eval_obj->computeMAP( *mqRetriever, APs, false, true, &evalParQueue_obj );
//         double mAP_b= bestSingle(APs);
        if (mqRetriever!=ret)
            delete mqRetriever;
        
        // best single
        APs.clear(); APs.resize(55,0);
        std::vector<double> thisAPs;
        
        for (uint32_t i=0; i< (googleQueries?numTop:5); ++i){
            std::cout<<i<<" "; std::cout.flush();
            singleRetToMQ mqBest(*ret, i);
            if (googleQueries)
                mqRetriever= new multiQueryFromWords( &mqBest, fidx_obj, numTop, true, usePrague );
            else
                mqRetriever= new multiQueryRetFake( &mqBest, eval_obj, fidx_obj );
            eval_obj->computeMAP( *mqRetriever, thisAPs, false, false, &evalParQueue_obj );
            delete mqRetriever;
            for (uint32_t iAP=0; iAP<APs.size(); ++iAP)
                APs[iAP]= std::max(APs[iAP], thisAPs[iAP]);
        }
        std::cout<<"\n";
        double mAP_b= 0;
        for (uint32_t iAP=0; iAP<APs.size(); ++iAP)
            mAP_b+= APs[iAP];
        mAP_b/=APs.size();
        
        
        printf("Single %.4f\n", mAP);
        printf("Best Single %.4f\n", mAP_b);
        
        ret= NULL;
        
    }
    
    delete eval_obj;
    delete iidx_obj;
    delete fidx_obj;
    delete tfidf_obj;
    
}
