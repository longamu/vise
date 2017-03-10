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

#ifndef _TFIDF_AUG_H_
#define _TFIDF_AUG_H_

#include <stdint.h>
#include <vector>

// for Arthur, similar as for api.h
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/split_member.hpp>

#include "bow_retriever.h"
#include "forward_index.h"
#include "iidx_wrapper_jp_gen.h"
#include "tfidf.h"
#include "image_graph.h"

class tfidfAug : public bowRetriever {
    
    public:
        
        tfidfAug( invertedIndex *aIidx, forwardIndex *aForwardIndex_obj, imageGraph &aImageGraph_obj, const char fileName[]= NULL ) : bowRetriever(aForwardIndex_obj), iidx(aIidx), imageGraph_obj(&aImageGraph_obj) {
            if (fileName==NULL)
                compute();
            else
                loadFromFile( fileName );
        }
        
        void
            queryExecute( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
        void
            queryExecute( represent &represent_obj, std::vector<double> &scores ) const;
        
        void
            compute( bool recomputeIdf= true );
        
        void
            saveToFile( const char fileName[] );
        
        void
            loadFromFile( const char fileName[] );
    
    private:
        
        invertedIndex *iidx;
        imageGraph *imageGraph_obj;
        std::vector<double> idfAug, docL2_idfAug, docL2aug;
    
    private:
        
        friend class boost::serialization::access;
        template<class A>
        void save(A & archive, const unsigned int version) const;
        template<class A>
        void load(A & archive, const unsigned int version);
        BOOST_SERIALIZATION_SPLIT_MEMBER()
        
};

#endif

