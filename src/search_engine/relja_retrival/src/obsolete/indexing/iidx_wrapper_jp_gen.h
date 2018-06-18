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

#ifndef _IIDX_WRAPPER_JP_GEN_H_
#define _IIDX_WRAPPER_JP_GEN_H_

#include <vector>
#include <map>

#include "forward_index.h"
#include "inverted_index.h"
#include "iidx_generic.hpp"
#include "iidx_builder.hpp"



template <class freqT>
class iidxWrapperJpGen : public invertedIndex {
    
    typedef iidx_generic<uint32_t, std::pair<uint32_t, freqT> > jp_iidx_generic_type;
    
    
    public:
        
        iidxWrapperJpGen( const char fileName[] );
        iidxWrapperJpGen( const char fileName[], forwardIndex &fidx, const char tmpDir[]= "" );
        
        virtual void
            getDocFreq( uint32_t wordID, std::vector< docFreqPair > &docFreq ) const;
        
        virtual uint32_t
            getNumDocContainingWord( uint32_t wordID );
        
        virtual uint32_t
            numWords() const { return jp_iidx_generic.max_key() +1; }
        
        virtual uint32_t
            numDocs() const { return jp_iidx_generic.max_data().first +1; }
    
    protected:
        
        jp_iidx_generic_type jp_iidx_generic;
    
};



template <class freqT>
iidxWrapperJpGen<freqT>::iidxWrapperJpGen( const char fileName[] ) :
        invertedIndex(), jp_iidx_generic(fileName,'r') {
}



template <class freqT>
iidxWrapperJpGen<freqT>::iidxWrapperJpGen( const char fileName[], forwardIndex &fidx, const char tmpDir[] ) :
        invertedIndex(), jp_iidx_generic(fileName,'w') {
    
    std::cout<<"iidxWrapperJpGen: computing the inverted index\n";
    
    // adapted from James's indexes.py
    
    std::vector<quantDesc> words;
    std::vector<ellipse> regions;
    std::vector<wordWeightPair> flat_words;
    
    iidx_builder<uint32_t, std::pair<uint32_t, freqT> > iidxBuilder_obj( tmpDir, 1 );
    uint32_t numDocs= fidx.numDocs();
    
    // add all words
    for (uint32_t docID= 0; docID < numDocs; ++docID){
        
        if (docID%1000==0)
            std::cout<<"iidxWrapperJpGen: adding docID= "<<docID<<"\n";
        
        fidx.getWordsRegs( docID, words, regions );
        quantDesc::flatten( words, flat_words );
        
        std::map<uint32_t,freqT> BoW;
        for (std::vector<wordWeightPair>::iterator itWW= flat_words.begin(); itWW!=flat_words.end(); ++itWW)
            BoW[ itWW->first ]+= itWW->second;
        
        for (typename std::map<uint32_t,freqT>::iterator itW= BoW.begin(); itW!=BoW.end(); ++itW)
            iidxBuilder_obj.add_term( itW->first, std::make_pair(docID, itW->second) );
        
    }
    
    // merge the iidx
    std::cout<<"iidxWrapperJpGen: merge index\n";
    iidxBuilder_obj.merge_till_one();
    
    // create the inverted index from the sorted output
    std::cout<<"iidxWrapperJpGen: create final index\n";
    std::string fn= iidxBuilder_obj.get_last_fn();
    jp_iidx_generic.load_from_fn(fn.c_str());
    remove( fn.c_str() );
    
    std::cout<<"iidxWrapperJpGen: computing the inverted index - DONE\n";
    
}



template <class freqT>
void
iidxWrapperJpGen<freqT>::getDocFreq( uint32_t wordID, std::vector< docFreqPair> &docFreq ) const {
    
    docFreq.clear();
    typename std::vector< std::pair<uint32_t,freqT> > terms;
    typename std::vector< std::pair<uint32_t,freqT> >::const_iterator begin, end;
    
    try {
        jp_iidx_generic.word_terms(wordID, terms);
    }
    catch(std::runtime_error e){
        // Word not found: just return empty vector, i.e. all fine
        return;
    }
    
    begin= terms.begin();
    end= terms.end();
    
    uint32_t numEntries= terms.size();
    docFreq.reserve( numEntries );
    
    for (; begin!=end; ++begin  ){
        docFreq.push_back( std::make_pair( begin->first, static_cast<float>(begin->second) ) );
    }
    
}



template <class freqT>
uint32_t
iidxWrapperJpGen<freqT>::getNumDocContainingWord( uint32_t wordID ){
    
    typename std::vector< std::pair<uint32_t,freqT> > terms;
    
    try {
        jp_iidx_generic.word_terms(wordID, terms);
        return terms.size();
    }
    catch(std::runtime_error e){
        // Word not found: just return 0, i.e. all fine
        return 0;
    }
    
}




#endif
