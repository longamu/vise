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

#include "fidx_v2_as_v1.h"

#include "index_entry_util.h"



void
fidxV2AsV1::getWordsRegs( uint32_t docID, std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const {
    std::vector<rr::indexEntry> entries;
    uint32_t n= idx_.getEntries( docID, entries );
    words.clear();
    regions.clear();
    words.reserve(n);
    regions.reserve(n);
    for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry){
        rr::indexEntry &entry= entries[iEntry];
        indexEntryUtil::unquantXY(entry);
        idx_.unquantEllipse(entry);
        for (int i=0; i<entry.id_size(); ++i){
            words.push_back( quantDesc(entry.id(i)) );
            regions.push_back( ellipse(entry.x(i), entry.y(i), entry.a(i), entry.b(i), entry.c(i)) );
        }
    }
}

