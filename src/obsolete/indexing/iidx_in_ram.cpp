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

#include "iidx_in_ram.h"

#include <boost/thread.hpp>

#include "iidx_generic_nonthreadsafe.hpp"

#include "timing.h"
#include "util.h"


iidxInRam::iidxInRam( invertedIndex &iidx ) : invertedIndex(), numDocs_(iidx.numDocs()), numWords_(iidx.numWords()) {
    
    docFreqs= new std::vector< docFreqPair >[ numWords_ ];
    
    uint32_t numWords_printStep= std::max(static_cast<uint32_t>(1),numWords_/20);
    
    std::cout<<"iidxInRam::iidxInRam: loading inverted index\n";
    double time= timing::tic();
    
    for (uint32_t wordID= 0; wordID<numWords_; ++wordID){
        
        if (wordID % numWords_printStep == 0)
            std::cout<<"iidxInRam::iidxInRam: loading wordID= "<<wordID<<" / "<<numWords_<<" "<<timing::toc(time)<<" ms\n";
        
        iidx.getDocFreq( wordID, docFreqs[wordID] );
        
    }
    
    std::cout<<"iidxInRam::iidxInRam: loading inverted index - DONE ("<<timing::toc(time)<<" ms)\n";
    
}



iidxInRam::iidxInRam( const char fileName[] ){
    
    // read the entire file - helps with loading very large files
    boost::thread t(boost::bind(util::visitFile,fileName));
    t.detach();
    
    typedef iidx_generic_nonthreadsafe<uint32_t, std::pair<uint32_t, uint32_t> > jpIidxType;
    jpIidxType iidx_obj(fileName,'r');
    
    numDocs_= iidx_obj.max_data().first +1;
    numWords_= iidx_obj.max_key() +1;
    
    docFreqs= new std::vector< docFreqPair >[ numWords_ ];
    
    uint32_t numWords_printStep= std::max(static_cast<uint32_t>(1),numWords_/20);
    
    std::cout<<"iidxInRam::iidxInRam: loading JP inverted index\n";
    double time= timing::tic();
    
    for (uint32_t wordID= 0; wordID<numWords_; ++wordID){
        
        if (wordID % numWords_printStep == 0)
            std::cout<<"iidxInRam::iidxInRam: loading wordID= "<<wordID<<" / "<<numWords_<<" "<<timing::toc(time)<<" ms\n";
        
        jpIidxType::const_iterator begin, end;
        
        try {
            begin= iidx_obj.begin(wordID);
        }
        catch(std::runtime_error e){
            // Word not found: just return empty vector, i.e. all fine
            continue;
        }
        
        end= iidx_obj.end(wordID);
        
        uint32_t numEntries= end-begin;
        docFreqs[wordID].reserve( numEntries );
        
        for (; begin!=end; ++begin  ){
            docFreqs[wordID].push_back( std::make_pair( begin->first, static_cast<float>(begin->second) ) );
        }
        
    }
    
    std::cout<<"iidxInRam::iidxInRam: loading JP inverted index - DONE ("<<timing::toc(time)<<" ms)\n";
    
}
