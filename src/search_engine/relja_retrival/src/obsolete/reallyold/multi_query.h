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

#ifndef _MULTI_QUERY_H_
#define _MULTI_QUERY_H_

#include "par_queue.h"
#include "thread_queue.h"
#include "bow_retriever.h"



class multiQuery {
    
    public:
        
        typedef std::vector<represent*> retsType;
        
        multiQuery() {}
        
        virtual
            ~multiQuery() {};
        
        virtual void
            query( retsType &represent_objs, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const = 0;
    
};



class multiQueryIndpt : public multiQuery {
    
    protected:
        
        typedef std::vector<indScorePair>* Result;
    
    public:
        
        multiQueryIndpt( bowRetriever const &aRetriever_obj ) : multiQuery(), retriever_obj(&aRetriever_obj) {}
        
        virtual
            ~multiQueryIndpt() {}
        
        virtual void
            query( retsType &represent_objs, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
        virtual bool
            workerReturnOnlyTop() const =0;
        
        virtual queueManager<Result>*
            getManager( std::vector<double> &aScores ) const =0;
        
    protected:
        
        bowRetriever const *retriever_obj;
        
    private:
        
        class mqIndpt_worker : public queueWorker<Result> {
            public:
                mqIndpt_worker( bowRetriever const &aRetriever_obj, retsType &aRepresent_objs, uint32_t aToReturn ) : retriever_obj(&aRetriever_obj), represent_objs(&aRepresent_objs), toReturn(aToReturn) {}
                void operator() ( uint32_t jobID, Result &result ) const;
            private:
                bowRetriever const *retriever_obj;
                retsType *represent_objs;
                const uint32_t toReturn;
        };
        
    
};



class multiQueryMax : public multiQueryIndpt {
    
    public:
        
        multiQueryMax( bowRetriever const &aRetriever_obj ) : multiQueryIndpt( aRetriever_obj ) {}
        
        bool
            workerReturnOnlyTop() const { return true; }
        
        queueManager<Result>*
            getManager( std::vector<double> &aScores ) const {
                return new mqMax_manager( aScores );
            };
    
    private:
    
        class mqMax_manager : public queueManager<Result> {
            public:
                mqMax_manager( std::vector<double> &aScores ) : scores(&aScores), first(true) {}
                void operator() ( uint32_t jobID, Result &result );
                std::vector<double> *scores;
            private:
                bool first;
        };
    
};



class multiQuerySum : public multiQueryIndpt {
    
    public:
        
        multiQuerySum( bowRetriever const &aRetriever_obj ) : multiQueryIndpt( aRetriever_obj ) {}
        
        bool
            workerReturnOnlyTop() const { return false; }
        
        queueManager<Result>*
            getManager( std::vector<double> &aScores ) const {
                return new mqSum_manager( aScores );
            };
    
    private:
    
        class mqSum_manager : public queueManager<Result> {
            public:
                mqSum_manager( std::vector<double> &aScores ) : scores(&aScores) {}
                void operator() ( uint32_t jobID, Result &result );
                std::vector<double> *scores;
        };
    
};



class multiQueryJointAvg : public multiQuery {
    
    public:
        
        multiQueryJointAvg( tfidf const &aTfidf_obj ) : multiQuery(), tfidf_obj(&aTfidf_obj) {}
        
        virtual
            ~multiQueryJointAvg() {}
        
        virtual void
            query( retsType &represent_objs, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
    protected:
        
        tfidf const *tfidf_obj;
    
};



class multiQueryPostSpatial : public multiQuery {
    
    public:
        
        multiQueryPostSpatial( multiQuery const &aFirstMultiQuery_obj, spatialVerif const &aSpatVerif_obj ) : multiQuery(), firstMultiQuery_obj(&aFirstMultiQuery_obj), spatVerif_obj(&aSpatVerif_obj) {}
        
        virtual
            ~multiQueryPostSpatial() {}
        
        virtual void
            query( retsType &represent_objs, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
    protected:
        
        multiQuery const *firstMultiQuery_obj;
        spatialVerif const *spatVerif_obj;
    
    private:
        
        class spatWorker : public queueWorker<double> {
            
            public:
                
                void operator() ( uint32_t resInd, double &result ) const {
                    
                    result= 0;
                    
                    double score= 0.0;
                    uint32_t numInliers;
                    uint32_t docID= queryRes->at(resInd).first;
                    
                    for (retsType::iterator itRep= represent_objs->begin(); itRep!=represent_objs->end(); ++itRep){
                        // should probably precopy..
                        represent thisRep= **itRep;
                        
                        homography H;
                        spatVerif_obj->getInliers( thisRep, docID, score, numInliers, H);
                        
                        if (numInliers >= minInliers)
                            result+= score;
                    }
                    
                }
                
                spatWorker( retsType &aRepresent_objs, spatialVerif const &aSpatVerif_obj, std::vector<indScorePair> const &aQueryRes ) : represent_objs(&aRepresent_objs), spatVerif_obj(&aSpatVerif_obj), queryRes(&aQueryRes), minInliers(aSpatVerif_obj.minInliers()) {}
            
            private:
                
                retsType *represent_objs;
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
