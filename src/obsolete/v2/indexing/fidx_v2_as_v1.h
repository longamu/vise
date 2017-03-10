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

#ifndef _FIDX_V2_AS_V1_H_
#define _FIDX_V2_AS_V1_H_

#include "forward_index.h"
#include "macros.h"
#include "proto_db.h"
#include "proto_index.h"



class fidxV2AsV1 : public forwardIndex {
    
    public:
        
        fidxV2AsV1(protoDb const &idxDb, uint32_t numDocs) : idx_(idxDb, true), numDocs_(numDocs) {}
        
        void
            getWordsRegs( uint32_t docID, std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const;
        
        uint32_t
            numDocs() const {
                return numDocs_;
            }
    
    private:
        
        protoIndex const idx_;
        uint32_t numDocs_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(fidxV2AsV1)
    
};

#endif
