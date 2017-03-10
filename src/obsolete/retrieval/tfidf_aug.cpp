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

#include "tfidf_aug.h"

#include <math.h>
#include <algorithm>
#include <fstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>



void
tfidfAug::queryExecute( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    std::vector<double> scores;
    queryExecute( represent_obj, scores );
    retriever::sortResults( scores, queryRes, toReturn );
}



void
tfidfAug::queryExecute( represent &represent_obj, std::vector<double> &scores ) const {
    
    tfidf::queryExecute( iidx, idfAug, docL2_idfAug, represent_obj, scores );
    std::vector<double> scores_noaug( scores );
    
    uint32_t docIDtoaug;
    std::vector<spatResType>::iterator itNeigh;
    
    for (imageGraph::imageGraphType::iterator itIG= imageGraph_obj->graph.begin(); itIG!= imageGraph_obj->graph.end(); ++itIG){
        
        docIDtoaug= itIG->first;
        std::vector<spatResType> &neighs= itIG->second;
        
        // careful with non-symmetric graphs - docl2 computation needs to be in the same direction (i.e. this+= neigh)
        for (itNeigh= neighs.begin(); itNeigh!=neighs.end(); ++itNeigh){
            scores[ docIDtoaug ]+= scores_noaug[ itNeigh->first.first ];
            // below not equivalent for non-symmetric graphs! (possible through imageGraph::keepVerified)
            // !! scores[ itNeigh->first.first ]+= scores_noaug[ docIDtoaug ];
        }
        
    }
    
    docIDtoaug= 0;
    for (std::vector<double>::iterator itS= scores.begin(); itS!=scores.end(); ++itS){
        // was already divided by |query| in tfidf::query
        (*itS) /= docL2aug[ docIDtoaug ];
        ++docIDtoaug;
    }
    
}



void
tfidfAug::compute( bool recomputeIdf ){
    
    idfAug.clear();
    docL2_idfAug.clear();
    docL2aug.clear();
    
    uint32_t numWords= iidx->numWords(), numDocs= iidx->numDocs();
    
    uint32_t docID;
    std::vector< docFreqPair > docFreq;
    
    uint32_t numWords_printStep= std::max(static_cast<uint32_t>(1),numWords/20);
    
    imageGraph::imageGraphType invGraph;
    imageGraph_obj->computeInverse( invGraph );
        
    //------- compute idf augmented
    
    if (recomputeIdf) {
        
        std::cout<<"tfidfAug::compute: computing augmented idfs\n";
        idfAug.resize( numWords, 0.0 );
        
        std::set<uint32_t> thisWordDocs;
        
        for (uint32_t wordID= 0; wordID < numWords; ++wordID){
            
            if (wordID % numWords_printStep == 0)
                std::cout<<"tfidfAug::compute: idfAug, wordID= "<<wordID<<" / "<<numWords<<"\n";
            
            iidx->getDocFreq( wordID, docFreq );
            
            thisWordDocs.clear();
            
            for (std::vector< docFreqPair >::iterator it= docFreq.begin(); it!=docFreq.end(); ++it){
                
                docID= it->first;
                thisWordDocs.insert( docID );
                
                // careful with non-symmetric graphs (see l2 computation)
                if ( invGraph.count( docID ) ) {
                    std::vector<spatResType> &invNeighs= invGraph.find( docID )->second;
                    
                    for (std::vector<spatResType>::iterator itInvNeigh= invNeighs.begin(); itInvNeigh!=invNeighs.end(); ++itInvNeigh){
                        thisWordDocs.insert( itInvNeigh->first.first );
                    }
                }
            
            }
            
            idfAug[ wordID ]= 
                log( 
                    static_cast<double>(numDocs) /
                    std::max( static_cast<uint32_t>(1), static_cast<uint32_t>(thisWordDocs.size()) )
                    );
            
        }
    
    } else {
        
        tfidf::computeIdf( iidx, idfAug );
        
    }
    
    
        
    //------- compute l2 length of each document
    
    {
    std::cout<<"tfidfAug::compute: computing document L2 lengths\n";
    // need to do this before augmented L2 and not in parallel as it uses this result
    tfidf::computeDocL2( iidx, idfAug, docL2_idfAug );
        
    std::cout<<"tfidfAug::compute: computing augmented document L2 lengths\n";
    docL2aug.resize( numDocs, 0.0 );
    std::vector<double> thisWordFreq; thisWordFreq.reserve(numDocs);
    
    double freqNorm;
    
    for (uint32_t wordID= 0; wordID < numWords; ++wordID){
        
        if (wordID % numWords_printStep == 0)
            std::cout<<"tfidfAug::compute: L2aug, wordID= "<<wordID<<" / "<<numWords<<"\n";
        
        iidx->getDocFreq( wordID, docFreq );
        
        thisWordFreq.clear(); thisWordFreq.resize( numDocs, 0.0 );
        
        for (std::vector< docFreqPair >::iterator it= docFreq.begin(); it!=docFreq.end(); ++it){
            
            docID= it->first;
            freqNorm= static_cast<double>(it->second) / docL2_idfAug[docID];
            
            thisWordFreq[ docID ]+= freqNorm;
            
            // careful with non-symmetric graphs - querying needs to be in the same direction (i.e. this+= neigh)
            if ( invGraph.count( docID ) ) {
                std::vector<spatResType> &invNeighs= invGraph.find( docID )->second;
                
                for (std::vector<spatResType>::iterator itInvNeigh= invNeighs.begin(); itInvNeigh!=invNeighs.end(); ++itInvNeigh){
                    thisWordFreq[ itInvNeigh->first.first ]+= freqNorm;
                }
            }
                        
        }
        
        // careful - don't only go through the documents returned by iidx!
        for (docID= 0; docID < numDocs; ++docID) {
            docL2aug[docID]+= pow( thisWordFreq[docID] * idfAug[wordID] , 2);
        }
        
    }
    
    for (uint32_t docID= 0; docID < numDocs; ++docID){
        if ( docL2aug[docID] <= 1e-7 )
            docL2aug[docID]= 1.0;
        else
            docL2aug[docID]= sqrt( docL2aug[docID] );
    }
    
    }
    
    std::cout<<"tfidfAug::compute: done\n";
    
}



void
tfidfAug::saveToFile( const char fileName[] ){
    
    std::ofstream ofs(fileName, std::ios::binary);
    boost::archive::binary_oarchive oa(ofs);
    oa << (*this);
    
}



void
tfidfAug::loadFromFile( const char fileName[] ){
    
    std::ifstream ifs(fileName, std::ios::binary);
    boost::archive::binary_iarchive ia(ifs);
    ia >> (*this);
    
}



template<class A>
void
tfidfAug::save(A & archive, const unsigned int version) const {
    uint32_t numWords= idfAug.size(), numDocs= docL2aug.size();
    archive & numWords & numDocs & idfAug & docL2_idfAug & docL2aug;
}



template<class A>
void
tfidfAug::load(A & archive, const unsigned int version){    
    uint32_t numWords, numDocs;
    archive & numWords & numDocs & idfAug & docL2_idfAug & docL2aug;
}

