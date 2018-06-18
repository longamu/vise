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

#include "weighter.h"

#include <math.h>



void
weighter::queryExecute( invertedIndex const *aIidx, std::map<uint32_t,double> const &wordWeight, std::vector<double> const &aIdf, std::vector<double> const &aDocL2, std::vector<double> &scores, double defaultScore ){
    
    scores.clear();
    scores.resize( aIidx->numDocs(), 0.0 );
    
    std::vector< docFreqPair > docFreq;
    std::vector< docFreqPair >::iterator itDF;
    
    double queryL2= 0.0, queryW= 0.0, widf;
    uint32_t wordID;
    
    for (std::map<uint32_t,double>::const_iterator itWW= wordWeight.begin();
         itWW!=wordWeight.end();
         ++itWW){
        
        // *itWW= (wordID,weight)
        wordID= itWW->first;
        
        queryW= itWW->second;
        widf= aIdf[wordID] * queryW;
        queryL2+= queryW * queryW;
        
        aIidx->getDocFreq( wordID, docFreq );
        
        for (itDF= docFreq.begin(); itDF!=docFreq.end(); ++itDF)
            scores[ itDF->first ]+= itDF->second * widf;
        
    }
    
    double queryL2sqrt= sqrt(queryL2);
    if (queryL2sqrt <= 1e-7)
        queryL2sqrt= 1.0;
    double defaultScoreByNorm= defaultScore / queryL2sqrt;
    
    std::vector<double>::const_iterator docL2Iter= aDocL2.begin();
    for (std::vector<double>::iterator itS= scores.begin(); itS!=scores.end(); ++itS, ++docL2Iter)
        (*itS)= (*itS) / ( queryL2sqrt * (*docL2Iter) ) + defaultScoreByNorm;
    
}



void
weighter::queryExecute( std::map<uint32_t,double> const &wordWeight, tfidf const *tfidf_obj, std::vector<double> &scores, double defaultScore ){
    queryExecute( tfidf_obj->iidx, wordWeight, tfidf_obj->idf, tfidf_obj->docL2, scores, defaultScore );
}
