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

#include "represent.h"

#include <stdint.h>
#include <vector>
#include <math.h>

#include "ellipse_util.h"



void
represent::computeBoW( std::vector<quantDesc> &aWords, std::map<uint32_t,double> &aBoW, std::vector<quantDesc>::iterator *begin ) {
    
    bool delBegin= (begin==NULL);
    if (delBegin){
        begin= new std::vector<quantDesc>::iterator( aWords.begin() );
    }
    
    aBoW.clear();
    
    std::vector< wordWeightPair >::const_iterator itWW;
    
    for (std::vector<quantDesc>::const_iterator itW= *begin; itW!=aWords.end(); ++itW)
        
        for (itWW= itW->rep.begin(); itWW!= itW->rep.end(); ++itWW)
            
            aBoW[ itWW->first ]+= itWW->second;
    
    if (delBegin)
        delete begin;
    
    l2Normalize( aBoW );
        
}



void
represent::l2Normalize( std::map<uint32_t,double> &aBoW ){
    // l2 normalize
    double l2= 0;
    std::map<uint32_t,double>::iterator itB;
    for (itB= aBoW.begin(); itB!=aBoW.end(); ++itB)
        l2+= pow(itB->second,2);
    l2= sqrt(l2);
    for (itB= aBoW.begin(); itB!=aBoW.end(); ++itB)
        itB->second /= l2;
}



void
represent::weight( std::map<uint32_t,double> &aBoW, std::vector<double> const &aWordWeight ){
    
    for (std::map<uint32_t,double>::iterator itB= aBoW.begin();
         itB!=aBoW.end();
         ++itB) {
        itB->second *= aWordWeight[ itB->first ];
    }
    
}



void
represent::filterROI(){
    
    // keep only ones in query region
    if ( query_obj->allInf() ){
        // all inside, TODO: instead check if all of the image is inside the query region
    } else {
        std::vector<quantDesc> wordsAll;
        std::vector<ellipse> regionsAll;
        words.swap(wordsAll);
        regions.swap(regionsAll);
        
        words.reserve( wordsAll.size() );
        regions.reserve( regionsAll.size() );
        std::vector<quantDesc>::const_iterator itW= wordsAll.begin();
        std::vector<ellipse>::const_iterator itR= regionsAll.begin();
        for (; itW!=wordsAll.end(); ++itW, ++itR){
            if ( ellipseUtil::inside( *itR, *query_obj ) ){
                regions.push_back( *itR );
                words.push_back( *itW );
            }
        }
    }
    
}



void
represent::load( forwardIndex const &forwardIndex_obj, bool doComputeBow ){
    
    forwardIndex_obj.getWordsRegs( query_obj->docID, words, regions );
    
    filterROI();
    
    if (doComputeBow)
        computeBoW( words, BoW );
    
}



void
represent::add( forwardIndex const &forwardIndex_obj, uint32_t docID, homography *H, bool addBoW ){
    
    // get words
    std::vector<quantDesc> wordsAll;
    std::vector<ellipse> regionsAll;
    forwardIndex_obj.getWordsRegs( docID, wordsAll, regionsAll );
    
    std::vector<quantDesc>::iterator newWordsBegin;
    
    // keep only ones which back project into the query region
    if ( query_obj->allInf() ){
        // should change inf stuff into query image size
#if 1
        std::cout<<"\tERROR: didn't implement this!!\n";
    } else {
#else
        double xu, yu;
        // HACK:
        xu= 0; yu= 0;
        for (std::vector<ellipse>::iterator itR= regions.begin(); itR!=regions.end(); ++itR){
            xu= (xu>itR->x)?xu:itR->x;
            yu= (yu>itR->y)?yu:itR->y;
        }
        query_obj->xl= 0;  query_obj->yl= 0;
        query_obj->xu= xu; query_obj->yu= yu;
    }{
#endif
        words.reserve( words.size()+wordsAll.size() ); // important to do this 1) should be faster 2) importantly: reallocation invalidates iterators, so newWordsBegin below would be invalid
        regions.reserve( regions.size()+regionsAll.size() );
        std::vector<quantDesc>::iterator itW= wordsAll.begin();
        std::vector<ellipse>::iterator itR= regionsAll.begin();
        
        newWordsBegin= words.end(); // important to do this after words.reserve!
        
        double Hinv[9];
        H->getInverse(Hinv);
        
        for (; itW!=wordsAll.end(); ++itW, ++itR){
            itR->transformAffine( Hinv );
            if ( ellipseUtil::inside( *itR, *query_obj ) ){
                regions.push_back( *itR );
                words.push_back( *itW );
            }
        }
    }
    
    if (addBoW) {
        
        std::map<uint32_t,double> BoW_expand;
        computeBoW( words, BoW_expand, &newWordsBegin );
        
        std::map<uint32_t,double>::iterator itBe;
        for (itBe= BoW_expand.begin(); itBe!=BoW_expand.end(); ++itBe)
            BoW[itBe->first] += itBe->second;
        
    }
    
}
