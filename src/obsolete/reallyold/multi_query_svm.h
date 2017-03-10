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

#ifndef _MULTI_QUERY_SVM_H_
#define _MULTI_QUERY_SVM_H_

#include "par_queue.h"
#include "thread_queue.h"
#include "forward_index.h"
#include "retriever.h"
#include "tfidf.h"
#include "spatial_verif.h"
#include "multi_query.h"



class multiQueryJointSVM : public multiQuery {
    
    public:
        
        multiQueryJointSVM( tfidf const &aTfidf_obj, forwardIndex const &aForwardIndex_obj );
        
        virtual
            ~multiQueryJointSVM() {}
        
        virtual void
            query( retsType &represent_objs, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
    protected:
        
        typedef std::map<uint32_t,double> BoWtype;
        std::vector<BoWtype> negatives;
        
        tfidf const *tfidf_obj;
        
        static const uint32_t numNeg= 200;
    
};



class exemplarSVM : public bowRetriever {
    
    public:
        
        // giving aSpatVerif_obj=NULL means no spatial verification and thus also calibration
        exemplarSVM( tfidf const &aTfidf_obj, forwardIndex *aForwardIndex_obj, spatialVerif const *aSpatVerif_obj= NULL, bool aDoCalibrate= true );
        
        void
            query( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
    
    protected:
        
        typedef std::map<uint32_t,double> BoWtype;
        std::vector<BoWtype> negatives;
        
        tfidf const *tfidf_obj;
        spatialVerif const *spatVerif_obj;
        std::set<uint32_t> calibNegDocIDs;
        
        bool doCalibrate;
        
        static const uint32_t numNeg= 200, numCalibNeg= 200;
        static const double scoreThr= 10.0;
    
    private:
        
        class spatWorker : public queueWorker<double> {
            
            public:
                
                void operator() ( uint32_t resInd, double &result ) const {
                    
                    result= 0;
                    
                    double score= 0.0;
                    uint32_t numInliers;
                    uint32_t docID= queryRes->at(resInd).first;
                    
                    homography H;
                    represent thisRep= represent_obj;
                    spatVerif_obj->getInliers( thisRep, docID, score, numInliers, H);
                    
                    if (numInliers >= minInliers)
                        result= score;
                    
                }
                
                spatWorker( represent const &aRepresent_obj, spatialVerif const &aSpatVerif_obj, std::vector<indScorePair> const &aQueryRes ) : represent_obj(aRepresent_obj), spatVerif_obj(&aSpatVerif_obj), queryRes(&aQueryRes), minInliers(aSpatVerif_obj.minInliers()) {}
            
            private:
                
                represent represent_obj;
                spatialVerif const *spatVerif_obj;
                std::vector<indScorePair> const *queryRes;
                uint32_t minInliers;
            
        };
        
        class spatManager : public queueManager<double> {
            
            public:
                void operator() ( uint32_t resInd, double &result ){
                    queryRes->at(resInd).second+= result;
                }
                spatManager( std::vector<indScorePair> &aQueryRes ) : queryRes(&aQueryRes) {}
            
            private:
                
                std::vector<indScorePair> *queryRes;
            
        };
    
};



#endif
