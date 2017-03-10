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

#ifndef _SPATIAL_VERIF_H
#define _SPATIAL_VERIF_H

#include <stdint.h>
#include <algorithm>

#include "par_queue.h"

#include "bow_retriever.h"
#include "homography.h"
#include "putative.h"
#include "spatial_defs.h"
#include "spatial_retriever.h"



class spatialVerif : public bowRetriever, public spatialRetriever {
    
    public:
        
        typedef std::pair<std::pair<double,uint32_t>,homography> Result;
        
        spatialVerif(bowRetriever const *aRetriever_obj,
                     forwardIndex const *aForwardIndexDB_obj= NULL,
                     spatParams *aSpatParams= NULL,
                     bool aSoftDB= false );
        
        virtual
            ~spatialVerif(){
                if (delSpatParams)
                    delete spatParams_obj;
            }
        
        virtual void
            queryExecute( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
        void
            spatialQuery( query const &query_obj, std::vector<indScorePair> &queryRes, std::map<uint32_t,homography> &Hs, uint32_t toReturn= 0 ) const;
        
        void
            spatialQuery( represent &represent_obj, std::vector<indScorePair> &queryRes, std::map<uint32_t,homography> &Hs, std::set<uint32_t> *ignoreDocs= NULL, uint32_t toReturn= 0, bool queryFirst= true ) const;
        
        void
            internalSpatialQuery( uint32_t docID, std::vector<spatResType> &querySpatRes, std::set<uint32_t> *ignoreDocs= NULL, uint32_t toReturn= 0 ) const;
        
        void
            getInliers( represent const &represent_obj, uint32_t docID, double &score, uint32_t &numInliers, homography &H, std::vector<uint32_t> const *presortedInds1= NULL, std::vector< std::pair<uint32_t,uint32_t> > *inlierInds= NULL ) const;
        
        void
            getMatches( query const &query_obj, uint32_t docID2, homography &H, std::vector< std::pair<ellipse,ellipse> > &matches ) const;
        
        static void
            spatResSort( std::vector<spatResType> &spatRes ){
                std::sort( spatRes.begin(), spatRes.end(), &spatialVerif::spatResCompare );
            }
        
        static bool
            spatResCompare( spatResType const &x, spatResType const &y ){
                return x.first.second > y.first.second;
            }
        
        inline uint32_t
            minInliers() const {
                return spatParams_obj->minInliers;
            }
        
        inline uint32_t
            spatialDepth() const {
                return spatParams_obj->spatialDepth;
            }
        
        inline void
            externalQuery_computeData( std::string imageFn, query const &queryObj ) const {
                firstRetriever_obj->externalQuery_computeData( imageFn, queryObj );
            }
        
        forwardIndex const *forwardIndexDB_obj;
        
    protected:
        
        bowRetriever const *firstRetriever_obj;
        spatParams *spatParams_obj;
        bool delSpatParams;
        bool softQ, softDB;
        
        
        class spatWorker : public queueWorker<Result> {
            public:
                
                inline void operator() ( uint32_t resInd, Result &result ) const {
                    if (doIgnoreDocs && ignoreDocs->count( queryRes->at(resInd).first ) ){
                        result.first.first= 0;
                        result.first.second= 0;
                        return;
                    }
                    parent->getInliers( *represent_obj, queryRes->at(resInd).first, result.first.first, result.first.second, result.second, &presortedInds1 );
                }
                
                spatWorker( spatialVerif const *aParent, represent &aRepresent_obj, std::vector<indScorePair> const &aQueryRes, bool useSoft, std::set<uint32_t> const *aIgnoreDocs ) : parent(aParent), represent_obj(&aRepresent_obj), queryRes(&aQueryRes), ignoreDocs(aIgnoreDocs) {
                    doIgnoreDocs= (ignoreDocs!=NULL) && (ignoreDocs->size()>0);
                    if (useSoft){
                        std::vector<wordWeightPair> ids1_flat;
                        std::vector<uint32_t> qDInds1;
                        quantDesc::flatten( represent_obj->words, ids1_flat, &qDInds1, true );
                        indSorter_Soft::sort( ids1_flat, presortedInds1 );
                    } else {
                        std::vector<uint32_t> ids1_flat;
                        quantDesc::flattenHard( represent_obj->words, ids1_flat );
                        indSorter_Hard::sort( ids1_flat, presortedInds1 );
                    }
                }
            private:
                spatialVerif const *parent;
                represent *represent_obj;
                std::vector<indScorePair> const *queryRes;
                std::set<uint32_t> const *ignoreDocs;
                bool doIgnoreDocs;
                std::vector<uint32_t> presortedInds1;
        };
        
        class spatManager : public queueManager<Result> {
            public:
                
                inline void operator() ( uint32_t resInd, Result &result ){
                    if ( result.first.second >= spatParams_obj->minInliers ){
                        if (addScore)
                            queryRes->at(resInd).second+= result.first.first;
                        else
                            queryRes->at(resInd).second= result.first.first;
                        (*Hs)[ queryRes->at(resInd).first ]= result.second;
                    }
                }
                
                spatManager( std::vector<indScorePair> &aQueryRes, spatParams const &aSpatParam_obj, std::map<uint32_t,homography> &aHs, bool aAddScore ) : queryRes(&aQueryRes), spatParams_obj(&aSpatParam_obj), Hs(&aHs), addScore(aAddScore) {}
            private:
                std::vector<indScorePair> *queryRes;
                spatParams const *spatParams_obj;
                std::map<uint32_t,homography> *Hs;
                bool const addScore;
                
        };
    
};

#endif
