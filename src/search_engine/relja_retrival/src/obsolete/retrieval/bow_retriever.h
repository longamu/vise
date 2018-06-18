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

#ifndef _BOW_RETRIEVER_H_
#define _BOW_RETRIEVER_H_

#include <stdint.h>
#include <vector>
#include <stdexcept>

#include <fastann/fastann.hpp>

#include "feat_getter.h"
#include "forward_index.h"
#include "query.h"
#include "represent.h"
#include "retriever.h"
#include "soft_assigner.h"


typedef std::pair<uint32_t,double> indScorePair;


class bowRetriever : public retriever {
    
    public:
        
        bowRetriever( forwardIndex const *aForwardIndex_obj,
                      featGetter const *aFeatGetter_obj= NULL,
                      fastann::nn_obj<float> const *aNn_obj= NULL,
                      softAssigner const *aSA_obj= NULL)
                      : forwardIndex_obj(aForwardIndex_obj),
                        featGetter_obj(aFeatGetter_obj),
                        nn_obj(aNn_obj),
                        SA_obj(aSA_obj),
                        numDims_(aFeatGetter_obj==NULL ? 0 : aFeatGetter_obj->numDims())
                      {}
        
        virtual
            ~bowRetriever(){};
        
        virtual void
            queryExecute( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const =0;
        
        virtual void
            queryExecute( query const &queryObj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
        uint32_t numDocs() const { return forwardIndex_obj->numDocs(); }
        
        void
            externalQuery_computeData( std::string imageFn, query const &queryObj ) const;
    
        
        forwardIndex const *forwardIndex_obj;
        
        featGetter const *featGetter_obj;
        fastann::nn_obj<float> const *nn_obj;
        softAssigner const *SA_obj;
        
    protected:
        
        friend class MQBM;
        represent*
            getRepresentFromQuery( query const &query_obj ) const;
        
        uint32_t numDims_;
    
};

#endif
