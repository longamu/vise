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

#ifndef _QUERY_EXPAND_SVM_H
#define _QUERY_EXPAND_SVM_H

#include <stdint.h>

#include "svm.h"

#include "retriever.h"
#include "spatial_verif.h"
#include "represent.h"
#include "query_expand.h"
#include "iidx_wrapper_jp_gen.h"
#include "weighter.h"
#include "tfidf.h"
#include "util.h"
#include "svm_util.h"


class queryExpandSVM : public queryExpand {
    
    public:
        
        typedef spatialVerif::Result Result;
        
        queryExpandSVM( invertedIndex *aIidx,
                        tfidf *aTfidf_obj,
                        spatialVerif &aSpatialVerif_obj,
                        uint32_t aTopVerified= 20,
                        double aScoreThr= 10.0,
                        bool aAdaptiveThr= true,
                        uint32_t aAdaptiveMinVerif= 4 ) :
            queryExpand( aSpatialVerif_obj, aTopVerified, aScoreThr, aAdaptiveThr, aAdaptiveMinVerif), iidx(aIidx), tfidf_obj(aTfidf_obj)
            {}
        
        static bool
            expandSVM( invertedIndex const *iidx,
                       forwardIndex const *forwardIndexDB_obj,
                       std::vector<double> &idf,
                       represent &represent_obj,
                       std::vector<indScorePair> &queryRes,
                       std::map<uint32_t,homography> &Hs,
                       bool isInternal,
                       uint32_t numVerif,
                       std::map<uint32_t,double> &w,
                       double &b );
        
    protected:
        
        typedef std::map<uint32_t,double> BoWtype;
        
        inline virtual bool
            stopIfNone() const { return false; }
        
        void
            expandAndQuery( represent &represent_obj,
                            std::vector<indScorePair> &queryRes,
                            std::map<uint32_t,homography> &Hs,
                            bool isInternal,
                            uint32_t numVerif,
                            std::set<uint32_t> &inPrepend,
                            uint32_t toReturn ) const;
        
        invertedIndex *iidx;
        tfidf *tfidf_obj;
            
};

#endif
