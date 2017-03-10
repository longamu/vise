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

#include "fidx_aug.h"

#ifdef AUG_BACKPROJECT
#include "ellipse_util.h"
#include "homography.h"
#include "query.h"
#endif



void
fidxAug::getWordsRegs( uint32_t docID, std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const {
    
    fidx->getWordsRegs( docID, words, regions );
    
    if (imageGraph_obj->graph.count(docID)==0)
        return;
    
    std::vector<spatResType> const &neighs= imageGraph_obj->graph.find(docID)->second;
    uint32_t docIDneigh;
    
    #ifdef AUG_BACKPROJECT
    homography const *H;
    double Hinv[9];
    double xu, yu;
    // HACK:
    xu= 0; yu= 0;
    for (std::vector<ellipse>::iterator itR= regions.begin(); itR!=regions.end(); ++itR){
        xu= (xu>itR->x)?xu:itR->x;
        yu= (yu>itR->y)?yu:itR->y;
    }
    query query_obj(docID, true, 0, xu, 0, yu);
    #endif
    
    for (std::vector<spatResType>::const_iterator itNeigh= neighs.begin();
         itNeigh!=neighs.end();
         ++itNeigh){
        
        docIDneigh= itNeigh->first.first;
        
        std::vector<quantDesc> wordsAug;
        std::vector<ellipse> regionsAug;
        fidx->getWordsRegs( docIDneigh, wordsAug, regionsAug );
        
        #ifndef AUG_BACKPROJECT
        words.insert( words.end(), wordsAug.begin(), wordsAug.end() );
        regions.insert( regions.end(), regionsAug.begin(), regionsAug.end() );
        #else
        
        words.reserve( words.size() + wordsAug.size() );
        regions.reserve( regions.size() + regionsAug.size() );
        std::vector<quantDesc>::iterator itW= wordsAug.begin();
        std::vector<ellipse>::iterator itR= regionsAug.begin();
        H= &(itNeigh->second);
        H->getInverse(Hinv);
        
        for (; itW!=wordsAug.end(); ++itW, ++itR){
            itR->transformAffine( Hinv );
            if ( ellipseUtil::inside( *itR, query_obj ) ){
                regions.push_back( *itR );
                words.push_back( *itW );
            }
        }
        
        #endif
        
    }
    
}
