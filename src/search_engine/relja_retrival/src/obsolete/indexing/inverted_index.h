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

#ifndef _INVERTED_INDEX_H_
#define _INVERTED_INDEX_H_

#include <stdint.h>
#include <vector>

#include <iostream>


typedef std::pair<uint32_t,float> docFreqPair;


class invertedIndex {
    
    public:
        
        virtual
            ~invertedIndex(){};
        
        virtual void
            getDocFreq( uint32_t wordID, std::vector< docFreqPair > &docFreq ) const =0;
                
        virtual uint32_t
            getNumDocContainingWord( uint32_t wordID ) const {
                std::vector< docFreqPair > docFreq;
                getDocFreq( wordID, docFreq );
                return docFreq.size();
            }
        
        virtual uint32_t
            numWords() const =0;
        
        virtual uint32_t
            numDocs() const =0;
        
        void
            test(){
                
                uint32_t wordID= 0;
                
                std::cout<<numWords()<<" "<<numDocs()<<"\n";
                
                std::vector< docFreqPair > docFreq;
                getDocFreq( wordID, docFreq );
                
                for (uint32_t i=0; i < docFreq.size(); ++i){
                    std::cout << "(" << docFreq[i].first << ", " << docFreq[i].second << ")\n";
                }
                
            }
    
};



#endif
