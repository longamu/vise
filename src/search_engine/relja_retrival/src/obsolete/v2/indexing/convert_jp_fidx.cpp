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

#include <boost/filesystem.hpp>

#include "argsort.h"
#include "dataset.h"
#include "document_map.h"
#include "fidx_wrapper_jp_db5.h"
#include "fidx_in_ram.h"
#include "index_entry.pb.h"
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "timing.h"
#include "quant_desc.h"



void sortWords(std::vector<quantDesc> &words, std::vector<ellipse> &regions) {
    // sort wordIDs
    std::vector<uint32_t> wordsFlat, sortInds;
    quantDesc::flattenHard(words, wordsFlat);
    argSort<uint32_t>::sort(wordsFlat, sortInds);
    
    // apply the sort
    std::vector<quantDesc> words_;
    std::vector<ellipse> regions_;
    words_.swap(words);
    regions_.swap(regions);
    words.reserve(words_.size());
    regions.reserve(regions_.size());
    for (uint32_t ind= 0; ind<sortInds.size(); ++ind){
        words.push_back(words_[ sortInds[ind] ]);
        regions.push_back(regions_[ sortInds[ind] ]);
    }
}

int main(){
   
    #if 1
        std::string dsetPrefix= "oxc1_5k";
        std::string dsetFn= "/home/relja/Relja/Data/oxc1_5k/dset_oxc1_5k.db";
        std::string fidxFn_old= "/home/relja/Relja/Data/oxc1_5k/word_oxc1_5k_hesaff_sift_hell_1000000_43.h5";
        std::string fidxFn_new= "/home/relja/Relja/Data/tmp/indexing_v2/fidx_oxc1_5k_hesaff_sift_hell_1000000_43.v2bin";
        documentMap docMap( dsetFn.c_str(), dsetPrefix.c_str() );
    #else
        std::string dsetPrefix= "BBCb";
        std::string dsetFn= "/home/relja/Relja/Data/BBC/BBCb/dset_BBCb.db";
        std::string docMapFn= "/home/relja/Relja/Data/BBC/BBCb/docMap.bin";
        std::string fidxFn_old= "/home/relja/Relja/Data/BBC/BBCb/word_BBCb_hesaff_rootsift_scale3_1000000_43.h5";
        std::string fidxFn_new= "/home/relja/Relja/Data/tmp/indexing_v2/fidx_BBCb_hesaff_rootsift_scale3_1000000_43.v2bin";
        
        if ( !boost::filesystem::exists( docMapFn ) ){
            documentMap docMap( dsetFn.c_str(), dsetPrefix.c_str() );
            docMap.saveToFile( docMapFn.c_str() );
        }
        dataset dset( dsetFn.c_str(), dsetPrefix.c_str() );
        documentMap docMap( docMapFn.c_str(), &dset );
    #endif
    
    #if 1
        fidxWrapperJpDb5_HardJP fidx_old(fidxFn_old.c_str(), docMap);
    #else
        fidxWrapperJpDb5_HardJP fidx_fold(fidxFn_old.c_str(), docMap);
        fidxInRam fidx_old(fidx_fold);
    #endif
    
    std::vector<quantDesc> words;
    std::vector<ellipse> regions;
    
    bool doDiff= true;
    bool quantXY= true;
    bool quantEl= true;
    
    // build, assumes hard quantization
    
    if (true) {
        std::cout<<"convertJpFidx: Building the index\n";
        protoDbFileBuilder dbBuilder(fidxFn_new, "testing");
        indexBuilder idxBuilder(dbBuilder, doDiff, quantXY, quantEl);
        uint32_t numDocs= fidx_old.numDocs();
        
        rr::indexEntry entry;
        
        for (uint32_t docID= 0; docID<numDocs; ++docID){
            if (docID%5000==0)
                std::cout<< "convertJpFidx: building "<< docID <<" / "<< numDocs <<"\n";
            
            fidx_old.getWordsRegs(docID, words, regions);
            
            if (doDiff)
                sortWords(words, regions);
            
            entry.Clear();
            for (uint32_t iFeat= 0; iFeat<words.size(); ++iFeat){
                ASSERT(words[iFeat].rep.size()==1);
                ASSERT( fabs(words[iFeat].rep[0].second-1) < 1e-6 );
                entry.add_id(words[iFeat].rep[0].first);
                entry.add_x(regions[iFeat].x);
                entry.add_y(regions[iFeat].y);
                entry.add_a(regions[iFeat].a);
                entry.add_b(regions[iFeat].b);
                entry.add_c(regions[iFeat].c);
            }
            idxBuilder.addEntry(docID, entry);
        }
    }
    
    // check
    
    if (false) {
        std::cout<<"convertJpFidx: Testing the index\n";
        #if 1
            protoDbFile db_new(fidxFn_new);
        #else
            protoDbFile db_fnew(fidxFn_new);
            protoDbInRam db_new(db_fnew);
        #endif
        
        protoIndex fidx_new(db_new);
        
        uint32_t numDocs= fidx_new.numIDs();
        std::cout<<numDocs<<"\n";
        ASSERT(numDocs == fidx_old.numDocs());
        
        std::vector<rr::indexEntry> entries;
        double t0= timing::tic();
        
        for (uint32_t docID= 0; docID<numDocs; ++docID){
            bool verbose= (docID%5000==0);
            if (verbose)
                std::cout<< "convertJpFidx: testing "<< docID <<" / "<< numDocs <<"\n";
            
            fidx_old.getWordsRegs(docID, words, regions);
            
            if (doDiff)
                sortWords(words, regions);
            
            uint32_t numEntries= fidx_new.getEntries(docID, entries);
            ASSERT(numEntries==words.size());
            
            uint32_t iFeat= 0;
            for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry){
                
                rr::indexEntry const &entry= entries[iFeat];
                ASSERT(entry.diffid_size()==0);
                if (!quantXY){
                    ASSERT(entry.id_size() == entry.x_size());
                    ASSERT(entry.id_size() == entry.y_size());
                } else {
                    ASSERT(entry.id_size() == entry.qx_size());
                    ASSERT(entry.id_size() == entry.qy_size());
                }
                if (!quantEl){
                    ASSERT(entry.id_size() == entry.a_size());
                    ASSERT(entry.id_size() == entry.b_size());
                    ASSERT(entry.id_size() == entry.c_size());
                } else {
                    ASSERT(static_cast<uint32_t>(entry.id_size()) == entry.qel_scale().length());
                    ASSERT(static_cast<uint32_t>(entry.id_size()) == entry.qel_ratio().length());
                    ASSERT(static_cast<uint32_t>(entry.id_size()) == entry.qel_angle().length());
                }
                
                for (int i=0; i<entry.id_size(); ++i){
                    ASSERT(entry.id(i) == words[iFeat].rep[0].first);
                    ++iFeat;
                }
            }
        }
        
        std::cout<< timing::toc(t0) <<"\n";
        std::cout<<"Test passed!\n";
    }
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
