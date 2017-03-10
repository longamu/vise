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

#ifndef _MULTI_QUERY_MQBM_H_
#define _MULTI_QUERY_MQBM_H_

#include <stdexcept>
#include <string>
#include <vector>

#include "bow_retriever.h"
#include "multi_query.h"



// from Fernando, ICCV 2013
// NOTE: assumes BoW retrieval
class MQBM : public multiQuery {
    
    public:
        
        MQBM( multiQuery const &mq, bowRetriever const &bowRet ) : mq_(&mq), bowRet_(&bowRet) {}
        
        virtual
            ~MQBM() {}
        
        void
            queryExecute( std::vector<query> const &queries, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
        uint32_t
            numDocs() const { return mq_->numDocs(); }
    
    private:
        
        multiQuery const *mq_;
        bowRetriever const *bowRet_;
};



#endif
