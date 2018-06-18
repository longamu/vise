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

#include "spatial_verif.h"

#include <algorithm>

#include "ellipse.h"
#include "det_ransac.h"
#include "util.h"
#include "thread_queue.h"
#include "same_random.h"
#include "macros.h"



spatialVerif::spatialVerif(
        bowRetriever const *aRetriever_obj,
        forwardIndex const *aForwardIndexDB_obj,
        spatParams *aSpatParams,
        bool aSoftDB)
        : bowRetriever( aRetriever_obj->forwardIndex_obj,
                        aRetriever_obj->featGetter_obj,
                        aRetriever_obj->nn_obj,
                        aRetriever_obj->SA_obj ),
          forwardIndexDB_obj(aForwardIndexDB_obj==NULL ?
                                aRetriever_obj->forwardIndex_obj :
                                aForwardIndexDB_obj),
          firstRetriever_obj( aRetriever_obj ),
          spatParams_obj( aSpatParams ),
          delSpatParams( aSpatParams==NULL ),
          softQ( aRetriever_obj->SA_obj != NULL ),
          softDB(aSoftDB) {
   
    
    if (spatParams_obj==NULL)
        spatParams_obj= new spatParams();
    
    //!!TODO check if db/query is soft
    
}



void
spatialVerif::queryExecute( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    std::map<uint32_t,homography> Hs;
    spatialQuery( represent_obj, queryRes, Hs, NULL, toReturn );
}



void
spatialVerif::spatialQuery( query const &query_obj, std::vector<indScorePair> &queryRes, std::map<uint32_t,homography> &Hs, uint32_t toReturn ) const {
    represent *represent_obj= getRepresentFromQuery(query_obj);
    spatialQuery( *represent_obj, queryRes, Hs, NULL, toReturn );
    delete represent_obj;
}



void
spatialVerif::spatialQuery( represent &represent_obj, std::vector<indScorePair> &queryRes, std::map<uint32_t,homography> &Hs, std::set<uint32_t> *ignoreDocs, uint32_t toReturn, bool queryFirst ) const {
    
    uint32_t spatialDepthEff= spatParams_obj->spatialDepth;
    
    if (ignoreDocs!=NULL)
        spatialDepthEff+= ignoreDocs->size();
    
    if (queryFirst){
        uint32_t toReturnFirst= toReturn;
        if (toReturn!=0 && toReturnFirst < spatialDepthEff )
            toReturnFirst= spatialDepthEff;
        firstRetriever_obj->queryExecute( represent_obj, queryRes, toReturnFirst );
    }
    
    spatWorker worker_obj( this, represent_obj, queryRes, softDB || softQ, ignoreDocs );
    spatManager manager_obj( queryRes, *spatParams_obj, Hs, !softDB && !softQ );
    
    threadQueue<spatialVerif::Result>::start(
        std::min( static_cast<uint32_t>( queryRes.size() ), spatialDepthEff ),
        worker_obj, manager_obj, 10
    );
    
    retriever::sortResults( queryRes, spatialDepthEff, toReturn );
    
}



void
spatialVerif::internalSpatialQuery( uint32_t docID, std::vector<spatResType> &querySpatRes, std::set<uint32_t> *ignoreDocs, uint32_t toReturn ) const {
    std::vector<indScorePair> queryRes;
    std::map<uint32_t,homography> Hs;
    
    query query_obj(docID, true);
    represent *represent_obj= getRepresentFromQuery(query_obj);
    spatialQuery( *represent_obj, queryRes, Hs, ignoreDocs, toReturn );
    delete represent_obj;
    
    querySpatRes.clear();
    for (std::vector<indScorePair>::iterator itR= queryRes.begin();
         itR!=queryRes.end() && Hs.count(itR->first);
         ++itR){
        
        querySpatRes.push_back( std::make_pair(*itR, Hs[itR->first]) );
        
    }
}



inline double mysqr( double x ){ return x*x; }



void
spatialVerif::getInliers( represent const &represent_obj, uint32_t docID, double &score, uint32_t &numInliers, homography &H, std::vector<uint32_t> const *presortedInds1, std::vector< std::pair<uint32_t,uint32_t> > *inlierInds ) const {
    
    bool delInliers= false;
    if (inlierInds==NULL){
        delInliers= true;
        inlierInds= new std::vector< std::pair<uint32_t,uint32_t> >();
    }
    
    uint32_t numQueryWords= represent_obj.words.size();
    std::vector<quantDesc>::const_iterator itWquery, itWres;
    
    std::vector<quantDesc> wordsRes;
    std::vector<ellipse> regionsRes;
    
    ASSERT(forwardIndexDB_obj!=NULL);
    forwardIndexDB_obj->getWordsRegs( docID, wordsRes, regionsRes );
    
    bool isQuery= false;
    
    // for speedup check if equal to query
    if ( wordsRes.size() == numQueryWords ){
        itWres= wordsRes.begin();
        for ( itWquery= represent_obj.words.begin();
              ( itWquery!=represent_obj.words.end() ) && ( *itWquery == *itWres );
              ++itWquery ){
            ++itWres;
        }
        if (itWquery==represent_obj.words.end()){
            // are equal (didn't check regions but what are the chances..)
            numInliers= numQueryWords;
            score= numInliers;
            H.setIdentity();
            inlierInds->clear();
            inlierInds->reserve(numInliers);
            uint32_t iRegion= 0;
            for (std::vector<ellipse>::iterator itR= regionsRes.begin(); itR!= regionsRes.end(); ++itR){
                inlierInds->push_back( std::make_pair(iRegion,iRegion) );
                ++iRegion;
            }
            isQuery= true;
        }
    }
    
    if (!isQuery) {
        if (softDB || softQ){
            score=
                detRansac::matchWords_Soft(
                            sameRandomObj_,
                            numInliers,
                            represent_obj.words, represent_obj.regions, presortedInds1,
                            wordsRes, regionsRes,
                            spatParams_obj->errorThr,
                            spatParams_obj->lowAreaChange, spatParams_obj->highAreaChange,
                            spatParams_obj->maxReest,
                            &H, inlierInds
                            );
        } else {
            score=
                detRansac::matchWords_Hard(
                            sameRandomObj_,
                            numInliers,
                            represent_obj.words, represent_obj.regions, presortedInds1,
                            wordsRes, regionsRes,
                            spatParams_obj->errorThr,
                            spatParams_obj->lowAreaChange, spatParams_obj->highAreaChange,
                            spatParams_obj->maxReest,
                            &H, inlierInds
                            );
        }
    }
    
    if (delInliers)
        delete inlierInds;
    
}



void
spatialVerif::getMatches( query const &query_obj, uint32_t docID2, homography &H, std::vector< std::pair<ellipse,ellipse> > &matches ) const {
    
    double score;
    uint32_t numInliers;
    std::vector< std::pair<uint32_t,uint32_t> > inlierInds;
    
    represent *represent_obj= getRepresentFromQuery(query_obj);
    getInliers( *represent_obj, docID2, score, numInliers, H, NULL, &inlierInds );
    
    uint32_t inInd1, inInd2;
    
    std::vector<quantDesc> wordsRes;
    std::vector<ellipse> regionsRes;
    forwardIndexDB_obj->getWordsRegs( docID2, wordsRes, regionsRes );
    
    matches.clear();
    matches.reserve( numInliers );
    
    for (uint32_t iIn=0; iIn<numInliers; ++iIn){
        
        inInd1= inlierInds[iIn].first;
        inInd2= inlierInds[iIn].second;
        
        matches.push_back( std::make_pair(
            represent_obj->regions[ inInd1 ],
            regionsRes[ inInd2 ] ) );
        
    }
    
    delete represent_obj;
    
}
