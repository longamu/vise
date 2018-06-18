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

#include "bow_retriever.h"

#include <stdexcept>
#include <fstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include "util.h"
#include "macros.h"



represent*
bowRetriever::getRepresentFromQuery( query const &queryObj ) const {
    
    represent *represent_obj;
    if (queryObj.isInternal){
        if (forwardIndex_obj==NULL)
            throw std::runtime_error("Haven't provided a forward index object!");
        represent_obj= new represent( *forwardIndex_obj, queryObj );
    } else {
        // load words and regions
        std::vector<quantDesc> words;
        std::vector<ellipse> regions;
        try {
            std::ifstream ifs(queryObj.compDataFn.c_str(), std::ios::binary);
            boost::archive::binary_iarchive ia(ifs);
            ia >> regions >> words;
        } catch (boost::archive::archive_exception &e) {
            std::cout<<"wordFn doesn't exist: "<<queryObj.compDataFn<<"\n";
        }
        
        represent_obj= new represent( queryObj, words, regions );
    }
    
    return represent_obj;
    
}



void
bowRetriever::queryExecute( query const &queryObj, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    
    represent *represent_obj= getRepresentFromQuery(queryObj);
    queryExecute( *represent_obj, queryRes, toReturn );
    delete represent_obj;
}



void
bowRetriever::externalQuery_computeData( std::string imageFn, query const &queryObj ) const {
    
    ASSERT( featGetter_obj!=NULL );
    ASSERT( nn_obj!=NULL );
    
    // compute features
    
    uint32_t numFeats;
    std::vector<ellipse> regions;
    float *descs;
    
    std::cout<<"Extracting features\n";
    featGetter_obj->getFeats(imageFn.c_str(), numFeats, regions, descs);
    std::cout<<"Extracting features - DONE\n";
    
    // get visual words
    
    std::vector<quantDesc> words;
    words.reserve( numFeats );
    
    for (uint32_t iFeat=0; iFeat<numFeats; ++iFeat){
        
        // assign to clusters
        
        uint KNN= (SA_obj==NULL) ? 1 : 3;
        unsigned *clusterID= new unsigned[KNN];
        float *distSq= new float[KNN];
        
        nn_obj->search_knn(descs+iFeat*numDims_, 1, KNN, clusterID, distSq);
        
        quantDesc ww;
        
        for (uint iNN=0; iNN < KNN; ++iNN)
            ww.rep.push_back( std::make_pair(clusterID[iNN], distSq[iNN]) );
        
        delete []clusterID;
        delete []distSq;
        
        // assign weights to visual words (soft or hard)
        
        if (SA_obj!=NULL && KNN>1)
            SA_obj->getWeights(ww);
        else
            ww.rep[0].second= 1.0f;
        
        words.push_back(ww);
        
    }
    
    // cleanup
    delete []descs;
    
    // save to file
    std::ofstream ofs(queryObj.compDataFn.c_str(), std::ios::binary);
    boost::archive::binary_oarchive oa(ofs);
    oa << regions << words;
    
}
