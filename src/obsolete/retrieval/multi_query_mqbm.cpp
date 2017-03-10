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

#include "multi_query_mqbm.h"

#include <stdexcept>
#include <fstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/filesystem.hpp>

#include "util.h"



void
MQBM::queryExecute( std::vector<query> const &queries, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    
    if ( queries.size() <= 1 ){
        // insufficient number of queries - skip MQBM
        
        mq_->queryExecute(queries, queryRes, toReturn);
        
    } else {
        
        std::vector<query> qMQBM= queries;
        
        std::vector<std::string> compDataFns(queries.size());
        
        std::map<uint32_t,bool> stableWord; // i.e. appears in >=2 query images
        
        std::vector<represent*> represent_objs;
        
        // get stable words
        
        for (uint32_t iQuery= 0; iQuery<qMQBM.size(); ++iQuery) {
            
            // load BoW
            query &query_obj= qMQBM[iQuery];
            represent_objs.push_back( bowRet_->getRepresentFromQuery(query_obj) );
            
            // record the words
            for (std::map<uint32_t,double>::const_iterator it= represent_objs[iQuery]->BoW.begin();
                 it != represent_objs[iQuery]->BoW.end();
                 ++it) {
                if (stableWord.count(it->first))
                    stableWord[it->first]= true;
                else
                    stableWord[it->first]= false;
            }
            
        }
        
        // save stable words for external queries (internal can't do in the current architecture)
        
        for (uint32_t iQuery= 0; iQuery<qMQBM.size(); ++iQuery) {
            
            query &query_obj= qMQBM[iQuery];
            
            if (!query_obj.isInternal) {
                
                std::vector<quantDesc> words;
                std::vector<ellipse> regions;
                
                std::vector<quantDesc>::const_iterator itW= represent_objs[iQuery]->words.begin();
                std::vector<ellipse>::const_iterator itR= represent_objs[iQuery]->regions.begin();
                std::vector< wordWeightPair >::const_iterator itWW;
                
                for (; itW != represent_objs[iQuery]->words.end(); ++itW, ++itR){
                    
                    // go through all soft assignments, keep if at least one is stable
                    bool oneStable= false;
                    for (itWW= itW->rep.begin(); itWW!= itW->rep.end(); ++itWW)
                        if (stableWord.count(itWW->first) && stableWord[itWW->first]) {
                            oneStable= true;
                            break;
                        }
                    
                    if (oneStable) {
                        // keep
                        words.push_back(*itW);
                        regions.push_back(*itR);
                    }
                    
                }
                
                // save to file
                
                query_obj.compDataFn= util::getTempFileName();
                
                std::ofstream ofs(query_obj.compDataFn.c_str(), std::ios::binary);
                boost::archive::binary_oarchive oa(ofs);
                oa << regions << words;
                
            }
            
        }
        
        // cleanup
        for (uint32_t i= 0; i<represent_objs.size(); ++i)
            delete represent_objs[i];
        stableWord.clear();
        
        
        // query
        
        mq_->queryExecute(qMQBM, queryRes, toReturn);
        
        
        // delete the extra files
        
        for (uint32_t iQuery= 0; iQuery<qMQBM.size(); ++iQuery)
            if (!qMQBM[iQuery].isInternal) {
                boost::filesystem::remove( qMQBM[iQuery].compDataFn );
            }
        
    }
    
}
