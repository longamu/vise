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

#include "iidx_with_data_builder.h"


#include <stdio.h>


#include "util.h"
#include "index_with_data_file.h"
#include "iidx_generic.hpp"
#include "iidx_builder.hpp"
#include "macros.h"



void
iidxWithDataBuilder::buildInter( indexWithData const &fidx, std::string fileName, std::string tmpDir ){
    
    std::cout<<"iidxWithDataBuilder::buildInter: computing the intermediate inverted index\n";
    
    iidx_builder<uint32_t, std::pair<uint32_t, uint32_t> > iidxBuilder_obj( tmpDir.c_str(), 1 );
    
    unsigned char *data;
    std::vector<uint32_t> vecIDs;
    uint32_t size;
    
    for (uint32_t docID= 0; docID < fidx.numIDs(); ++docID){
        
        if (docID%100000==0)
            std::cout<<"iidxWithDataBuilder::buildInter: adding docID= "<<docID<<"\n";
        
        fidx.getData(docID, vecIDs, data, size);
        delete []data;
        
        for (uint32_t i= 0; i<vecIDs.size(); ++i)
            iidxBuilder_obj.add_term( vecIDs[i], std::make_pair(docID, i) );
        
    }
    
    // merge the iidx
    std::cout<<"iidxWithDataBuilder::buildInter: merge intermediate index\n";
    iidxBuilder_obj.merge_till_one();
    
    // create the inverted index from the sorted output
    std::cout<<"iidxWithDataBuilder::buildInter: create final intermediate index\n";
    std::string fn= iidxBuilder_obj.get_last_fn();
    iidx_generic<uint32_t, std::pair<uint32_t, uint32_t> > jp_iidx_generic(fileName.c_str(), 'w');
    jp_iidx_generic.load_from_fn(fn.c_str());
    remove( fn.c_str() );
    
}



void
iidxWithDataBuilder::build( indexWithData const &fidx, std::string fileName, compressorIndep const &comp, std::string tmpDir ){
    
    std::string fileName_inter= fileName+ ".inter";
    iidxWithDataBuilder::buildInter( fidx, fileName_inter, tmpDir );
    
    uint32_t numCodePerVector= comp.numCodePerVector();
    
    // convert the temp index into the final one
    std::cout<<"iidxWithDataBuilder::build: create final index\n";
    
    iidx_generic<uint32_t, std::pair<uint32_t, uint32_t> > jp_iidx_generic(fileName_inter.c_str(), 'r');
    indexWithDataFileBuilder iidxBuilder_obj( fileName );
    
    uint32_t maxVecID= jp_iidx_generic.max_key() +1;
    
    std::vector<uint32_t> docIDs;
    
    for (uint32_t vecID= 0; vecID < maxVecID; ++vecID){
        
        try {
            
            std::vector< std::pair<uint32_t,uint32_t> > terms;
            jp_iidx_generic.word_terms(vecID, terms);
            
            docIDs.clear(); docIDs.reserve( terms.size() );
            charStream *charStreamOut= comp.charStreamFactoryCreate();
            charStreamOut->reserve( terms.size()*numCodePerVector );
            
            for (std::vector< std::pair<uint32_t,uint32_t> >::const_iterator it= terms.begin(); it!=terms.end(); ++it){
                
                // add docID
                docIDs.push_back( it->first );
                
                // add data corresponding to this descriptor, but reading it from fidx and adding to iidx data
                
                unsigned char *data_fidx;
                uint32_t size;
                std::vector<uint32_t> vecIDs_fidx;
                fidx.getData( it->first, vecIDs_fidx, data_fidx, size );
                ASSERT( vecIDs_fidx[it->second]==vecID );
                charStream *charStreamIn= comp.charStreamFactoryCreate();
                charStreamIn->setDataCopy( std::string(
                    reinterpret_cast<const char*>(&data_fidx[0]),
                    size) );
                
                for (uint32_t iCode= 0; iCode < numCodePerVector; ++iCode){
                    charStreamIn->setIter(it->second * numCodePerVector + iCode);
                    charStreamOut->add( charStreamIn->getNextUnsafe() );
                }
                
                delete charStreamIn;
                delete []data_fidx;
            }
            
            std::string data= charStreamOut->getDataCopy();
            iidxBuilder_obj.addData( vecID, docIDs,
                                     reinterpret_cast<uint8_t const*>(data.c_str()),
                                     data.size() );
            delete charStreamOut;
            
        } catch (std::runtime_error e){
            // VecID not found, fine just continue
        }
        
    }
    
    iidxBuilder_obj.close();
    
    remove(fileName_inter.c_str());
    
}
