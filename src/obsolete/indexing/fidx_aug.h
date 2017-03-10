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

#ifndef _FIDX_AUG_H_
#define _FIDX_AUG_H_

#include <vector>
#include <stdint.h>

#include "forward_index.h"
#include "image_graph.h"

#define AUG_BACKPROJECT



class fidxAug : public forwardIndex {
    
    public:
        
        fidxAug( forwardIndex *aFidx, imageGraph const *aImageGraph_obj ) : fidx(aFidx), imageGraph_obj(aImageGraph_obj)
            {}
        
        virtual ~fidxAug() {}
        
        void
            getWordsRegs( uint32_t docID, std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const;
        
        inline uint32_t
            numDocs() const { return fidx->numDocs(); }
    
    protected:
        
        forwardIndex *fidx;
        imageGraph const *imageGraph_obj;
        
};

#endif
