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

#include "iidx_v2_as_v1.h"



void
iidxV2AsV1::getDocFreq( uint32_t wordID, std::vector< docFreqPair > &docFreq ) const {
    std::vector<rr::indexEntry> entries;
    uint32_t n= idx_.getEntries( wordID, entries );
    docFreq.clear();
    docFreq.reserve(n);
    uint32_t currID, prevID= 0;
    uint32_t count;
    for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry){
        rr::indexEntry const &entry= entries[iEntry];
        bool hasCount= (entry.count_size()>0);
        for (int i=0; i<entry.id_size(); ++i){
            currID= entry.id(i);
            count= hasCount ? entry.count(i) : 1 ;
            if (i!=0 && currID==prevID)
                docFreq.back().second+= count;
            else
                docFreq.push_back(std::make_pair(currID, count));
            prevID= currID;
        }
    }
}
