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

#include "query_expand.h"



queryExpand::queryExpand(
        spatialVerif &aSpatialVerif_obj,
        uint32_t aTopVerified,
        double aScoreThr,
        bool aAdaptiveThr,
        uint32_t aAdaptiveMinVerif )
        : bowRetriever(aSpatialVerif_obj.forwardIndex_obj,
                       aSpatialVerif_obj.featGetter_obj,
                       aSpatialVerif_obj.nn_obj,
                       aSpatialVerif_obj.SA_obj),
          spatialVerif_obj(&aSpatialVerif_obj),
          forwardIndexDB_obj( aSpatialVerif_obj.forwardIndexDB_obj ),
          topVerified_(aTopVerified), adaptiveMinVerif_(aAdaptiveMinVerif),
          scoreThr(aScoreThr), adaptiveThr(aAdaptiveThr)  {
}



void
queryExpand::queryExecute( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    
    std::map<uint32_t,homography> Hs;
    spatialVerif_obj->spatialQuery( represent_obj, queryRes, Hs, NULL, 0 ); // toReturn != topVerified in case nothing to expand with or QE SVM!
    
    bool isInternal= represent_obj.isInternal();
    uint32_t topVerified= topVerified_ + isInternal;
    uint32_t adaptiveMinVerif= adaptiveMinVerif_ + isInternal;
    
    // count number of verified
    uint32_t numVerif, numPrepend;
    for (numVerif= isInternal; numVerif < queryRes.size() && queryRes[numVerif].second > scoreThr; ++numVerif);
    numPrepend= numVerif;
    numVerif= std::min( numVerif, topVerified );
    
    if (adaptiveThr && numVerif < adaptiveMinVerif ){
        for (; numVerif < topVerified && numVerif < queryRes.size() && queryRes[numVerif].second >= scoreThr/2.0; ++numVerif );
    }
    
    if (stopIfNone() && (numVerif <= isInternal))
        // nothing to expand with
        return;
    
    uint32_t docIDexpand;
    std::vector<indScorePair> prepend;
    std::set<uint32_t> inPrepend;
    prepend.reserve( numPrepend );
    
    for (uint32_t iRes= 0; iRes < numPrepend; ++iRes){
        docIDexpand= queryRes[iRes].first;
        prepend.push_back( std::make_pair( docIDexpand, 100000.0+queryRes[iRes].second ) );
        inPrepend.insert( docIDexpand );
    }
    
    expandAndQuery( represent_obj, queryRes, Hs, isInternal, numVerif, inPrepend, toReturn );
        
    // remove results to prepend from the new results
    for (uint32_t iRes= 0; iRes < queryRes.size(); ++iRes)
        if ( inPrepend.count( queryRes[iRes].first ) ) {
            queryRes.erase( queryRes.begin()+iRes );
            --iRes;
        }
    
    // prepend verified results
    queryRes.insert( queryRes.begin(), prepend.begin(), prepend.end() );
    
}



void
queryExpand::expandAndQuery( represent &represent_obj, std::vector<indScorePair> &queryRes, std::map<uint32_t,homography> &Hs, bool isInternal, uint32_t numVerif, std::set<uint32_t> &inPrepend, uint32_t toReturn ) const {
    
    uint32_t docIDexpand;
    
    for (uint32_t iRes= isInternal; iRes < numVerif; ++iRes){
        docIDexpand= queryRes[iRes].first;
        represent_obj.add( *forwardIndexDB_obj, docIDexpand, &Hs[docIDexpand], true );
    }
    
    // TODO careful about Hs not to lose the prepend ones
    spatialVerif_obj->spatialQuery( represent_obj, queryRes, Hs, &inPrepend, toReturn );
    
}
