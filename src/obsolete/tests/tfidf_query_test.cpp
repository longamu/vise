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

#include "fidx_wrapper_jp_db5.h"
#include "iidx_wrapper_jp_gen.h"
#include "bow_retriever.h"
#include "tfidf.h"
#include "document_map.h"

#include <iostream>
#include <vector>
#include <stdint.h>

int main(){
    
    documentMap docMap( "/home/relja/Relja/Code/relja_retrieval/temp/dset_oxc1_5k.db", "oxc1_5k" );
    iidxWrapperJpGen<uint32_t> *iidx_obj= new iidxWrapperJpGen<uint32_t>( "/home/relja/Relja/Code/relja_retrieval/temp/iidx_oxc1_5k_hesaff_sift_1000000_43.bin" );
    fidxWrapperJpDb5_HardJP *fidx_obj= new fidxWrapperJpDb5_HardJP("/home/relja/Relja/Code/relja_retrieval/temp/word_oxc1_5k_hesaff_sift_1000000_43.h5", docMap);
    
    char fn[] = "/home/relja/Relja/Code/relja_retrieval/temp/tfidf.ser.bin";
    retriever *tfidf_obj= new tfidf( iidx_obj, fidx_obj, fn );
    
    std::vector<indScorePair> queryRes;
    
    const char hash[]= "60fd73963102f86baf08325631f8912db34acba7fb46cc9a41b818099276187e";
    uint32_t docID= docMap.h2i( hash );
    
    query query_obj(docID, true, "", 136.5, 648.5, 34.1, 955.7 );
    tfidf_obj->queryExecute( query_obj, queryRes, 0 );
    
    delete tfidf_obj;
    delete iidx_obj;
    delete fidx_obj;
    
    std::cout<< "query: "<< hash <<"\n\n";
    
    for (int i=0; i<5; ++i){
        std::cout<< i <<" "<< queryRes[i].second <<" "<< docMap.i2h( queryRes[i].first ) <<"\n";
    }
    
}
