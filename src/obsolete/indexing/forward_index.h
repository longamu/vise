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

#ifndef _FORWARD_INDEX_H_
#define _FORWARD_INDEX_H_

#include "ellipse.h"
#include "quant_desc.h"

class forwardIndex {
    
    public:
        
        virtual void
            getWordsRegs( uint32_t docID, std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const =0;
        
        virtual uint32_t
            numDocs() const =0;
        
        virtual ~forwardIndex() {}
    
};

#endif
