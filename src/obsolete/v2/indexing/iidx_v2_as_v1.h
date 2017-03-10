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

#ifndef _IIDX_V2_AS_V1_H_
#define _IIDX_V2_AS_V1_H_

#include "inverted_index.h"
#include "macros.h"
#include "proto_db.h"
#include "proto_index.h"



class iidxV2AsV1 : public invertedIndex {
    
    public:
        
        iidxV2AsV1(protoDb const &idxDb, uint32_t numDocs) : idx_(idxDb, false), numDocs_(numDocs) {}
        
        void
            getDocFreq( uint32_t wordID, std::vector< docFreqPair > &docFreq ) const;
        
        inline uint32_t
            getNumDocContainingWord( uint32_t wordID ) const {
                return idx_.getNumWithID(wordID);
            }
        
        inline uint32_t
            numWords() const {
                return idx_.numIDs();
            }
        
        inline uint32_t
            numDocs() const {
                return numDocs_;
            }
    
    private:
        
        protoIndex const idx_;
        uint32_t numDocs_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(iidxV2AsV1)
};

#endif
