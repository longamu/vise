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

#include <fstream>

#include "dataset.h"
#include "dataset_v2.h"
#include "util.h"



int main(){
    
    datasetBuilder dB( util::expandUser("~/Relja/Data/axes/trecvid2012/ins/dset_trecvid2012ins.v2bin") );
    std::string findPath= "/users/relja/Relja/Databases/trecvid2012/ins";
    dataset dset( util::expandUser("~/Relja/Data/axes/trecvid2012/ins/dset_trecvid2012ins.db").c_str(), "trecvid2012ins", findPath );
    
    uint32_t const numDocs= dset.getNumDoc();
    std::string imageFn;
    std::pair<uint32_t, uint32_t> wh;
    
    for (uint32_t docID= 0; docID<numDocs; ++docID){
        imageFn= dset.getFn(docID);
        wh= dset.getWidthHeight(docID);
        if (docID<5)
            std::cout<<docID<<" "<<wh.first<<" "<<wh.second<<" "<<imageFn<<"\n";
        dB.add( imageFn, wh.first, wh.second );
    }
    dB.close();
    
    google::protobuf::ShutdownProtobufLibrary();
}
