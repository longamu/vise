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

#include "tfidf.h"

#include <math.h>
#include <fstream>
#include <algorithm>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>



void
tfidf::queryExecute( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    std::vector<double> scores;
    queryExecute( represent_obj, scores );
    retriever::sortResults( scores, queryRes, toReturn );
}



void
tfidf::queryExecute( invertedIndex const *aIidx, std::vector<double> const &aIdf, std::vector<double> const &aDocL2, represent &represent_obj, std::vector<double> &scores ) {
    
    // weight query BoW with idf
    
    std::map<uint32_t,double> BoWidf= represent_obj.BoW;
    
    for (std::map<uint32_t,double>::iterator itW= BoWidf.begin();
         itW!=BoWidf.end();
         ++itW){
        
        itW->second *= aIdf[ itW->first ];
        
    }
    
    // query
    
    weighter::queryExecute( aIidx, BoWidf, aIdf, aDocL2, scores );
    
}



void
tfidf::computeIdf( invertedIndex const *aIidx, std::vector<double> &aIdf ){
    
    uint32_t numWords= aIidx->numWords(), numDocs= aIidx->numDocs();
    
    aIdf.clear();
            
    // compute idf (pretend a word appears in 1 document if it appears in 0 to prevent division by 0)
    
    aIdf.resize( numWords, 0.0 );
    
    uint32_t numWords_printStep= std::max(static_cast<uint32_t>(1),numWords/20);
    
    std::cout<<"tfidf::compute: computing idfs\n";
    
    double time= timing::tic();
    
    for (uint32_t wordID= 0; wordID < numWords; ++wordID){
        
        if (wordID % numWords_printStep == 0)
            std::cout<<"tfidf::compute: idf, wordID= "<<wordID<<" / "<<numWords<<" "<<timing::toc(time)<<" ms\n";
        
        aIdf[ wordID ]= 
            log( 
                static_cast<double>(numDocs) /
                std::max( static_cast<uint32_t>(1), aIidx->getNumDocContainingWord( wordID ) )
                );
        
    }
    
    std::cout<<"tfidf::compute: computing idfs - DONE ("<<timing::toc(time)<<" ms)\n";
    
}



void
tfidf::computeDocL2( invertedIndex const *aIidx, std::vector<double> const &aIdf, std::vector<double> &aDocL2 ) {
    
    uint32_t numWords= aIidx->numWords(), numDocs= aIidx->numDocs();
    
    aDocL2.clear();
    aDocL2.resize( numDocs, 0.0 );
    std::vector< docFreqPair > docFreq;
    
    uint32_t numWords_printStep= std::max(static_cast<uint32_t>(1),numWords/20);
    
    std::cout<<"tfidf::compute: computing document L2 lengths\n";
    double time= timing::tic();
    
    for (uint32_t wordID= 0; wordID < numWords; ++wordID){
        if (wordID % numWords_printStep == 0)
            std::cout<<"tfidf::compute: L2, wordID= "<<wordID<<" / "<<numWords<<" "<<timing::toc(time)<<" ms\n";
        aIidx->getDocFreq( wordID, docFreq );
        for (std::vector< docFreqPair >::iterator it= docFreq.begin(); it!=docFreq.end(); ++it){
            aDocL2[ it->first ]+= pow( it->second * aIdf[wordID] , 2);
        }
    }
    
    for (uint32_t docID= 0; docID < numDocs; ++docID){
        if ( aDocL2[docID] <= 1e-7 )
            aDocL2[docID]= 1.0;
        else
            aDocL2[docID]= sqrt( aDocL2[docID] );
    }
    
    std::cout<<"tfidf::compute: computing document L2 lengths - DONE ("<<timing::toc(time)<<" ms)\n";
    
}



void
tfidf::saveToFile( const char fileName[] ){
    
    std::ofstream ofs(fileName, std::ios::binary);
    boost::archive::binary_oarchive oa(ofs);
    oa << (*this);
    
}



void
tfidf::loadFromFile( const char fileName[] ){
    
    std::ifstream ifs(fileName, std::ios::binary);
    boost::archive::binary_iarchive ia(ifs);
    ia >> (*this);
    
}



template<class A>
void
tfidf::save(A & archive, const unsigned int version) const {
    uint32_t numWords= idf.size(), numDocs= docL2.size();
    archive & numWords & numDocs & idf & docL2;
}



template<class A>
void
tfidf::load(A & archive, const unsigned int version){    
    uint32_t numWords, numDocs;
    archive & numWords & numDocs & idf & docL2;
}

