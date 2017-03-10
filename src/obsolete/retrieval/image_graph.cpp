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

#include "image_graph.h"

#include <iostream>
#include <fstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>



imageGraph::imageGraph( const char fileName[], uint32_t numDocs, spatialVerif &spatialVerif_obj, uint32_t maxNeighs, double scoreThr, std::set<uint32_t> *excludes ) {
    
    graph.clear();
    
    bool limitNeighs= (maxNeighs>0);
    double score;
    uint32_t numNeighsTotal= 0, numNeighs, docIDres;
    std::vector<spatResType> querySpatRes;
    
    for (uint32_t docID=0; docID < numDocs; ++docID){
        
        if ( excludes==NULL || excludes->count(docID)==0 ) {
        
            querySpatRes.clear();
            spatialVerif_obj.internalSpatialQuery( docID, querySpatRes, NULL, maxNeighs );
            numNeighs= 0;
            
            std::vector<spatResType> neighs;
            if (limitNeighs)
                neighs.reserve( maxNeighs );
            
            for (std::vector<spatResType>::iterator itRes= querySpatRes.begin(); itRes!=querySpatRes.end(); ++itRes){
                
                docIDres= itRes->first.first;
                score= itRes->first.second;
                
                if (docIDres == docID)
                    continue;
                
                if (score >= scoreThr && !(limitNeighs && numNeighs >= maxNeighs) ){
                    neighs.push_back( *itRes );
                    ++numNeighs;
                } else
                    break;
                
            }
            
            if (numNeighs > 0){
                graph.insert( std::make_pair(docID, neighs) );
            }
            
            numNeighsTotal+= numNeighs;
            
        }
        
        if (docID % 100 == 0 || docID == numDocs-1){
            std::cout<< "imageGraph: ["<<(docID+1) <<"] neigh per image= "<< static_cast<double>(numNeighsTotal)/(docID+1)<<"\n";
        }
        
    }
    
    removeExcludes(excludes);
    
    makeSymmetric();
    
    saveToFile( fileName );
    
}



void
imageGraph::makeSymmetric(){
    
    uint32_t docIDthis, docIDthat;
    uint32_t iNeigh, jNeigh;
    imageGraphType::iterator itIGthat;
    double scoreThis, scoreThat;
    
    uint32_t numAdded= 0, progress_= 0;
    
    for (imageGraphType::iterator itIG= graph.begin(); itIG!= graph.end(); ++itIG){
        
        docIDthis= itIG->first;
        std::vector<spatResType> &neighs= itIG->second;
        
        for (iNeigh=0; iNeigh < neighs.size(); ++iNeigh){
            
            docIDthat= neighs[iNeigh].first.first;
            scoreThis= neighs[iNeigh].first.second;
            
            // find docIDthat node
            if ( graph.count( docIDthat )==0 ){
                // has no neighbours - insert empty list
                graph.insert( std::make_pair(docIDthat, std::vector<spatResType>() ) );
            }
            itIGthat= graph.find( docIDthat ); // we definitely have something
            
            // get neighbours of that
            std::vector<spatResType> &neighsThat= itIGthat->second;
            
            // find if docIDthat contains docIDthis
            for (jNeigh=0;
                 (jNeigh < neighsThat.size()) && (neighsThat[jNeigh].first.first!=docIDthis ); ++jNeigh );
            
            if (jNeigh < neighsThat.size()) {
                // does contain: keep largest score
                scoreThat= neighsThat[jNeigh].first.second;
                if (scoreThat<scoreThis){
                    neighsThat[jNeigh].first.second= scoreThis;
                    neighs[iNeigh].second.getInverse( neighsThat[jNeigh].second );
                }
                else {
                    neighs[iNeigh].first.second= scoreThat;
                    neighsThat[jNeigh].second.getInverse( neighs[iNeigh].second );
                }
            } else {
                // doesn't contain: add
                homography Hinv;
                neighs[iNeigh].second.getInverse( Hinv );
                neighsThat.push_back(
                    std::make_pair( std::make_pair( docIDthis, scoreThis ), Hinv )
                );
                ++numAdded;
            }
        }
        
        if ( progress_ % 1000==0 || progress_==graph.size()-1 ){
            std::cout<< "imageGraph: ["<<(progress_+1) <<"] numAdded per image= "<< static_cast<double>(numAdded)/(progress_+1)<<"\n";
        }
        ++progress_;
        
    }
    
    // resort everything as we might have ended up with an unsorted list of neighbours
    for (imageGraphType::iterator itIG= graph.begin(); itIG!= graph.end(); ++itIG){
        std::vector<spatResType> &neighs= itIG->second;
        spatialVerif::spatResSort( neighs );
    }
    
}



void
imageGraph::computeInverse( imageGraphType &invGraph ){
    
    invGraph.clear();
    
    uint32_t docIDthis, docIDthat;
    uint32_t iNeigh;
    double score;
    
    for (imageGraphType::iterator itIG= graph.begin(); itIG!= graph.end(); ++itIG){
        
        docIDthis= itIG->first;
        std::vector<spatResType> &neighs= itIG->second;
        
        for (iNeigh=0; iNeigh < neighs.size(); ++iNeigh){
            
            docIDthat= neighs[iNeigh].first.first;
            score= neighs[iNeigh].first.second;
            homography Hinv;
            neighs[iNeigh].second.getInverse( Hinv );
            
            invGraph[ docIDthat ].push_back(
                std::make_pair( std::make_pair( docIDthis, score ), Hinv )
                );
            
        }
        
    }
    
    // resort everything as we might have ended up with an unsorted list of neighbours
    for (imageGraphType::iterator itInvIG= invGraph.begin(); itInvIG!= invGraph.end(); ++itInvIG){
        std::vector<spatResType> &neighs= itInvIG->second;
        spatialVerif::spatResSort( neighs );
    }
    
}



void
imageGraph::keepVerified( uint32_t maxNeighs, double scoreThr, bool adaptiveThr, uint32_t adaptiveMinVerif, bool forceSymmetric ) {
    
    bool limitNeighs= (maxNeighs>0);
    uint32_t numVerif= 0;
    double scoreThr_adapt= scoreThr/2.0;
    
    uint32_t numBefore= 0, numAfter= 0;
    
    for (imageGraphType::iterator itIG= graph.begin(); itIG!= graph.end(); ++itIG){
        
        std::vector<spatResType> &neighs= itIG->second;
        
        numBefore+= neighs.size();
        
        for (numVerif=0;
             (numVerif < neighs.size()) && (neighs[numVerif].first.second >= scoreThr);
             ++numVerif);
        
        if (adaptiveThr){
            if (numVerif < adaptiveMinVerif){
                for (;
                     (numVerif < neighs.size()) && (neighs[numVerif].first.second >= scoreThr_adapt);
                     ++numVerif);
            }
        }
        
        if (limitNeighs && (numVerif > maxNeighs))
            numVerif= maxNeighs;
        
        if (numVerif < neighs.size()){
            neighs.erase( neighs.begin()+numVerif, neighs.end() );
        }
        
        numAfter+= neighs.size();
        
    }
    
    std::cout<< "imageGraph::keepVerified: nNeigh was= "<<numBefore<<" is= "<<numAfter<<"\n";
    
    if (forceSymmetric) {
        makeSymmetric();
        std::cout<< "imageGraph::keepVerified: Made symmetric after keepVerified\n";
    }
    
}



void
imageGraph::removeExcludes( std::set<uint32_t> *excludes ){
    
    if (excludes==NULL) return;
    
    uint32_t erasedQueryNodes= 0, erasedNeighNodes= 0;;
    
    // remove excludes as queries
    for (std::set<uint32_t>::iterator itE= excludes->begin(); itE!=excludes->end(); ++itE)
        erasedQueryNodes+= graph.erase( *itE );
    
    // remove excludes as neighbours
    for (imageGraphType::iterator itIG= graph.begin(); itIG!= graph.end(); ++itIG){
        
        std::vector<spatResType> &neighs= itIG->second;
        
        if (neighs.size()==0) continue;
        for (std::vector<spatResType>::iterator itN= neighs.end()-1; ; --itN){
            
            if ( excludes->count(itN->first.first) ){
                ++erasedNeighNodes;
                itN= neighs.erase(itN);
            }
            
            if (itN==neighs.begin()) break;
            
        }
        
    }
    
    if (erasedQueryNodes>0 || erasedNeighNodes>0) {
        std::cout<<"\n\t\tWARNING\nimageGraph::removeExcludes number of erased nodes>0 -- SHOULDN'T HAVE HAPPENED, check code for image graph creation or make sure you didn't use an old graph (pre 110523) which had bugs\n";
        if (erasedQueryNodes>0)
            std::cout<<"imageGraph::removeExcludes: erasedQueryNodes= "<<erasedQueryNodes<<"\n";
        if (erasedNeighNodes>0)
            std::cout<<"imageGraph::removeExcludes: erasedNeighNodes= "<<erasedNeighNodes<<"\n";
    }
    
}



void
imageGraph::saveToFile( const char fileName[] ){
    
    std::cout<<"\nimageGraph::saveToFile: fileName= "<<fileName<<"\n";
    std::ofstream ofs(fileName, std::ios::binary);
    boost::archive::binary_oarchive oa(ofs);
    oa << (*this);
    
}



void
imageGraph::loadFromFile( const char fileName[] ){
    
    std::cout<<"\nimageGraph::loadFromFile: fileName= "<<fileName<<"\n";
    std::ifstream ifs(fileName, std::ios::binary);
    boost::archive::binary_iarchive ia(ifs);
    ia >> (*this);
    
}
