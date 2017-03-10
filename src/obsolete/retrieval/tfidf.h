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

#ifndef _TFIDF_H_
#define _TFIDF_H_

#include <stdint.h>
#include <vector>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/split_member.hpp>

#include "bow_retriever.h"
#include "iidx_wrapper_jp_gen.h"
#include "forward_index.h"
#include "weighter.h"
#include "timing.h"

class weighter;

class tfidf : public bowRetriever {
    
    public:
        
        tfidf( invertedIndex *aIidx,
               forwardIndex *aForwardIndex_obj,
               const char fileName[]= NULL,
               // for BoW retriever
               featGetter const *aFeatGetter_obj= NULL,
               fastann::nn_obj<float> const *aNn_obj= NULL,
               softAssigner const *aSA_obj= NULL)
            : bowRetriever(aForwardIndex_obj, aFeatGetter_obj, aNn_obj, aSA_obj), iidx(aIidx) {
            if (fileName==NULL)
                compute();
            else
                loadFromFile( fileName );
        }
        
        virtual
            ~tfidf() {}
        
        virtual void
            queryExecute( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
        inline void
            queryExecute( represent &represent_obj, std::vector<double> &scores ) const {
                queryExecute( iidx, idf, docL2, represent_obj, scores );
            }
        
        static void
            queryExecute( invertedIndex const *aIidx, std::vector<double> const &aIdf, std::vector<double> const &aDocL2, represent &represent_obj, std::vector<double> &scores );
        
        inline void
            compute(){
                double time= timing::tic();
                computeIdf( iidx, idf );
                computeDocL2( iidx, idf, docL2 );
                std::cout<<"tfidf::compute - DONE ("<<timing::toc(time)<<" ms)\n";
            }
        
        static void
            computeIdf( invertedIndex const *aIidx, std::vector<double> &aIdf );
        
        static void
            computeDocL2( invertedIndex const *aIidx, std::vector<double> const &aIdf, std::vector<double> &aDocL2 );
        
        void
            saveToFile( const char fileName[] );
        
        void
            loadFromFile( const char fileName[] );
        
        invertedIndex *iidx;
        std::vector<double> idf, docL2;
    
    private:
        
        friend class boost::serialization::access;
        template<class A>
        void save(A & archive, const unsigned int version) const;
        template<class A>
        void load(A & archive, const unsigned int version);
        BOOST_SERIALIZATION_SPLIT_MEMBER()
    
};

#endif

