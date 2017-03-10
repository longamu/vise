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

#ifndef _QUERY_EXPAND_H
#define _QUERY_EXPAND_H

#include <stdint.h>

#include "retriever.h"
#include "spatial_verif.h"
#include "represent.h"


class queryExpand : public bowRetriever {
    
    public:
        
        typedef spatialVerif::Result Result;
        
        virtual
            ~queryExpand() {}
        
        queryExpand( spatialVerif &aSpatialVerif_obj,
                     uint32_t aTopVerified= 20,
                     double aScoreThr= 10.0,
                     bool aAdaptiveThr= true,
                     uint32_t aAdaptiveMinVerif= 4 );
        
        void
            queryExecute( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
    protected:
        
        inline virtual bool
            stopIfNone() const { return true; }
        
        virtual void
            expandAndQuery( represent &represent_obj,
                            std::vector<indScorePair> &queryRes,
                            std::map<uint32_t,homography> &Hs,
                            bool isInternal,
                            uint32_t numVerif,
                            std::set<uint32_t> &inPrepend,
                            uint32_t toReturn ) const;
        
        spatialVerif const *spatialVerif_obj;
        forwardIndex const *forwardIndexDB_obj;
        const uint32_t topVerified_, adaptiveMinVerif_;
        double const scoreThr;
        bool const adaptiveThr;
            
};

#endif
