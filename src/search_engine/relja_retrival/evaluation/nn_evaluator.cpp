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

#include "nn_evaluator.h"

#include "util.h"
#include "timing.h"
#include "nn_searcher.h"
#include "desc_from_fvecs_file.h"
#include "macros.h"

#include "thread_queue.h"

#include <stdio.h>





void readIvecs( const char fn[], uint32_t &num, uint32_t** &v );



nnEvaluator::nnEvaluator(std::string queryFn) {
    
    // load gt
    readIvecs(  util::expandUser("~/Relja/Data/Jegou_ANN/ANN_SIFT1M/sift_groundtruth.ivecs").c_str(), nQueries, gt );
    
    // load query vectors
    descFromFvecsFile queryFile( util::expandUser(queryFn).c_str() );
    ASSERT( nQueries==queryFile.numDocs() );
    numDims= queryFile.numDims();
    
    uint32_t numDescs_;
    qVecs= new float*[nQueries];
    
    for (uint32_t queryID= 0; queryID<nQueries; ++queryID){
        
        qVecs[queryID]= new float[numDims];
        queryFile.getDescs(queryID, numDescs_, qVecs[queryID]);
        ASSERT( numDescs_==1 );
        
    }
    
}



nnEvaluator::~nnEvaluator() {
    
    util::del( nQueries, qVecs );
    util::del( nQueries, gt );
    
}


class computeRecWorker : public queueWorker<nnEvaluator::recResultType> {
    public:
        computeRecWorker( nnEvaluator const &aNnEvaluator_obj, nnSearcher const &aNnSearcher_obj, uint32_t aRecallAt ) : nnEvaluator_obj(&aNnEvaluator_obj), nnSearcher_obj(&aNnSearcher_obj), recallAt(aRecallAt)
            {}
        void operator() ( uint32_t queryID, nnEvaluator::recResultType &result ) const {
            double queryTime;
            double recall= nnEvaluator_obj->computeRecall( queryID, *nnSearcher_obj, recallAt, queryTime );
            result= std::make_pair( recall, queryTime );
        }
    private:
        nnEvaluator const *nnEvaluator_obj;
        nnSearcher const *nnSearcher_obj;
        uint32_t recallAt;
};


class computeRecManager : public queueManager<nnEvaluator::recResultType> {
    public:
        computeRecManager( std::vector<double> &aRecs, uint32_t aNQueries, bool aVerbose, bool aSemiVerbose ) : recs(&aRecs), nQueries(aNQueries), verbose(aVerbose), semiVerbose(aSemiVerbose), averageRec(0.0), cumRec(0.0), time(0.0), times(aNQueries,-1.0), nextToPrint(0) {
            recs->clear();
            recs->resize(nQueries,0);
        }
        void operator()( uint32_t queryID, nnEvaluator::recResultType &result ){
            double rec= result.first, queryTime= result.second;
            averageRec+= rec;
            time+= queryTime;
            recs->at(queryID)= rec;
            times[queryID]= queryTime;
            if ( (verbose || semiVerbose) && nextToPrint==queryID){
                for (; nextToPrint<nQueries && times[nextToPrint]>-0.5; ++nextToPrint){
                    cumRec+= recs->at(nextToPrint);
                    if (verbose || (semiVerbose && nextToPrint%200==0))
                        printf("%.4d %.0f %.2f ms %.4f\n", nextToPrint, recs->at(nextToPrint), times[nextToPrint], cumRec/(nextToPrint+1) );
                }
            }
        }
        std::vector<double> *recs;
        uint32_t nQueries;
        bool verbose, semiVerbose;
        double averageRec, cumRec, time;
        std::vector<double> times;
        uint32_t nextToPrint;
};



double
nnEvaluator::computeAverageRec( nnSearcher const &nnSearcher_obj, uint32_t recallAt, std::vector<double> *recs, bool verbose, bool semiVerbose ) const {
    
    semiVerbose= semiVerbose && !verbose;
    
    bool deleteRecs= false;
    if (recs==NULL){
        deleteRecs= true;
        recs= new std::vector<double>();
    }
    
    
    if ( verbose || semiVerbose)
        printf("i recall@%d time avgrecall\n\n", recallAt);
        
    computeRecWorker computeRecWorker_obj( *this, nnSearcher_obj, recallAt );
    computeRecManager computeRecManager_obj( *recs, nQueries, verbose, semiVerbose );
    threadQueue<recResultType>::start( nQueries, computeRecWorker_obj, computeRecManager_obj, 0 );
    
    double rec= computeRecManager_obj.averageRec / nQueries;
    double time= computeRecManager_obj.time;
    
    if ( verbose || semiVerbose)
        printf("\n\trecall@%d= %.4f, time= %.4f s, avgTime= %.4f ms\n\n", recallAt, rec, time/1000, time/nQueries);
    
    if (deleteRecs)
        delete recs;
    
    return rec;
    
}

double
nnEvaluator::computeRecall( uint32_t queryID, nnSearcher const &nnSearcher_obj, uint32_t recallAt, double &time ) const {
    
    uint32_t posID= gt[queryID][0];
    
    std::vector<nnSearcher::vecIDdist> vecIDdists;
    
    // query
    time= timing::tic();
    nnSearcher_obj.findKNN( qVecs[queryID], recallAt, vecIDdists );
    time= timing::toc( time );
    
    ASSERT( recallAt >= vecIDdists.size() );
    if ( recallAt > vecIDdists.size() ){
        std::cout<<"warning: didn't return enough enough for recall@"<<recallAt<<" ("<<vecIDdists.size()<<")\n";
    }
    
    std::vector<nnSearcher::vecIDdist>::const_iterator it= vecIDdists.begin();
    for ( ; it!=vecIDdists.end() && it->ID!=posID; ++it);
    
    return it!=vecIDdists.end();
    
}



void
readIvecs( const char fn[], uint32_t &num, uint32_t** &v ){
    
    FILE *f= fopen(fn,"rb");
    
    int d, d_;
    size_t temp_= fread(&d, sizeof(int), 1, f);
    fseeko64(f, 0, SEEK_END );
    num= ftello64(f)/((d+1)*4);
    fseeko64(f, 0, SEEK_SET);
    v= new uint32_t*[num];
    for (uint32_t i=0; i<num; ++i){
        v[i]= new uint32_t[d];
        temp_= fread(&d_, sizeof(float), 1, f);
        ASSERT( round(d_)==d ); // check dimension is ok
        temp_= fread(v[i], sizeof(uint32_t), d, f);
    }
    fclose(f);
    
    if (false && temp_) {} // to avoid the warning about not checking temp_
}
