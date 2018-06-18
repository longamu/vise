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

#include <stdint.h>
#include <string>
#include <vector>

#include "iidx_wrapper_jp_gen.h"
#include "iidx_in_ram.h"
#include "index_entry.pb.h"
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "timing.h"



int main(){
    
    #if 1
        std::string iidxFn_old= "/home/relja/Relja/Data/oxc1_5k/iidx_oxc1_5k_hesaff_sift_hell_1000000_43.bin";
        std::string iidxFn_new= "/home/relja/Relja/Data/tmp/indexing_v2/iidx_oxc1_5k_hesaff_sift_hell_1000000_43.v2bin";
    #else
        std::string iidxFn_old= "/home/relja/Relja/Data/BBC/BBCb/iidx_BBCb_hesaff_rootsift_scale3_1000000_43.bin";
        std::string iidxFn_new= "/home/relja/Relja/Data/tmp/indexing_v2/iidx_BBCb_hesaff_rootsift_scale3_1000000_43.v2bin";
    #endif
    
    #if 1
        iidxWrapperJpGen<uint32_t> iidx_old(iidxFn_old.c_str());
    #else
        iidxWrapperJpGen<uint32_t> iidx_fold(iidxFn_old.c_str());
        iidxInRam iidx_old(iidx_fold);
    #endif
    
    std::vector< docFreqPair > docFreqs;
    
    // build
    
    if (true) {
        std::cout<<"convertJpIidx: Building the index\n";
        protoDbFileBuilder dbBuilder(iidxFn_new, "testing");
        indexBuilder idxBuilder(dbBuilder, true);
        uint32_t numWords= iidx_old.numWords();
        
        rr::indexEntry entry;
        
        for (uint32_t wordID= 0; wordID<numWords; ++wordID){
            if (wordID%50000==0)
                std::cout<< "convertJpIidx: building "<< wordID <<" / "<< numWords <<"\n";
            
            iidx_old.getDocFreq(wordID, docFreqs);
            
            entry.Clear();
            for (uint32_t iDF= 0; iDF<docFreqs.size(); ++iDF){
                entry.add_id(docFreqs[iDF].first);
                entry.add_count(docFreqs[iDF].second);
            }
            idxBuilder.addEntry(wordID, entry);
        }
    }
    
    // check
    
    if (false) {
        std::cout<<"convertJpIidx: Testing the index\n";
        #if 1
            protoDbFile db_new(iidxFn_new);
        #else
            protoDbFile db_fnew(iidxFn_new);
            protoDbInRam db_new(db_fnew);
        #endif
        
        protoIndex iidx_new(db_new);
        
        uint32_t numWords= iidx_new.numIDs();
        std::cout<<numWords<<"\n";
        ASSERT(numWords == iidx_old.numWords());
        
        std::vector<rr::indexEntry> entries;
        double t0= timing::tic();
        
        for (uint32_t wordID= 0; wordID<numWords; ++wordID){
            bool verbose= (wordID%50000==0);
            if (verbose)
                std::cout<< "convertJpIidx: testing "<< wordID <<" / "<< numWords <<"\n";
            
            iidx_old.getDocFreq(wordID, docFreqs);
            uint32_t numEntries= iidx_new.getEntries(wordID, entries);
            ASSERT(numEntries==docFreqs.size());
            
            uint32_t iDF= 0;
            for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry){
                rr::indexEntry const &entry= entries[iDF];
                ASSERT(entry.diffid_size()==0);
                for (int i=0; i<entry.id_size(); ++i){
                    ASSERT(entry.id(i) == docFreqs[iDF].first);
                    ASSERT(entry.count(i) == docFreqs[iDF].second);
                    ++iDF;
                }
            }
        }
        
        std::cout<< timing::toc(t0) <<"\n";
        std::cout<<"Test passed!\n";
    }
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
