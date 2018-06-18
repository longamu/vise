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

#ifndef _IIDX_IN_RAM_H_
#define _IIDX_IN_RAM_H_

#include <boost/function.hpp>

#include "inverted_index.h"
#include "slow_construction.h"



class iidxInRam : public invertedIndex {
    
    public:
        
        iidxInRam( invertedIndex &iidx );
        
        // load from jp inverted index which is faster (but not thread safe)
        iidxInRam( const char fileName[] );
        
        virtual ~iidxInRam(){
            delete []docFreqs;
        }
        
        virtual void
            getDocFreq( uint32_t wordID, std::vector< docFreqPair > &docFreq ) const {
                docFreq= docFreqs[wordID];
            }
        
        virtual uint32_t
            getNumDocContainingWord( uint32_t wordID ) const {
                return docFreqs[wordID].size();
            }
        
        virtual uint32_t
            numWords() const { return numWords_; }
        
        virtual uint32_t
            numDocs() const { return numDocs_; }
    
    private:
        
        std::vector< docFreqPair > *docFreqs;
        uint32_t numDocs_, numWords_;
    
};



// Same as iidxInRam, but as loading this might take a long time,
// this class starts providing the underlying functionality immediately
// by accessing the iidx from disk and switching to the iidxInRam version
// once it is loaded. If multiple things are loaded from disk, use the
// sequentialConstructions class.
class iidxInRamStartDisk : public invertedIndex {
    
    public:
        
        iidxInRamStartDisk( invertedIndex const &iidx, boost::function<invertedIndex*()> iidxInRamConstructor, bool deleteFirst= false, sequentialConstructions *consQueue= NULL ) : slowCons_(&iidx, iidxInRamConstructor, deleteFirst, consQueue) {
        }
        
        inline void
            getDocFreq( uint32_t wordID, std::vector< docFreqPair > &docFreq ) const {
                slowCons_.getObject()->getDocFreq(wordID, docFreq);
            }
        
        inline uint32_t
            getNumDocContainingWord( uint32_t wordID ) const {
                return slowCons_.getObject()->getNumDocContainingWord(wordID);
            }
        
        inline uint32_t
            numWords() const { return slowCons_.getObject()->numWords(); }
        
        inline uint32_t
            numDocs() const { return slowCons_.getObject()->numDocs(); }
    
    private:
        
        slowConstruction<invertedIndex const> slowCons_;
};

#endif
