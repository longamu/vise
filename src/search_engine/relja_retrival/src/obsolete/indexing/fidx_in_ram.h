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

#ifndef _FIDX_IN_RAM_H_
#define _FIDX_IN_RAM_H_

#include <boost/function.hpp>

#include "forward_index.h"
#include "slow_construction.h"
#include "util.h"



class fidxInRam : public forwardIndex {
    
    public:
        
        fidxInRam( forwardIndex const &fidx );
        
        virtual ~fidxInRam(){
            delete []num;
            util::del(numDocs_, wordIDs);
            util::del(numDocs_, x);
            util::del(numDocs_, y);
            util::del(numDocs_, a);
            util::del(numDocs_, b);
            util::del(numDocs_, c);
        }
        
        virtual void
            getWordsRegs( uint32_t docID, std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const {
                words.clear();
                regions.clear();
                if (docID>=numDocs_)
                    return;
                words.reserve(num[docID]);
                regions.resize(num[docID]);
                for (uint32_t i= 0; i<num[docID]; ++i){
                    words.push_back(quantDesc(wordIDs[docID][i]));
                    ellipse &E= regions[i];
                    E.x= x[docID][i];
                    E.y= y[docID][i];
                    E.a= a[docID][i];
                    E.b= b[docID][i];
                    E.c= c[docID][i];
                }
            }
        
        virtual uint32_t
            numDocs() const { return numDocs_; }
    
    private:
        
        uint32_t *num;
        uint32_t **wordIDs;
        float **x, **y, **a, **b, **c;
        uint32_t numDocs_;
    
};



// See comments in iidx_in_ram for iidxInRamStartDisk
class fidxInRamStartDisk : public forwardIndex {
    
    public:
        
        fidxInRamStartDisk( forwardIndex const &fidx, boost::function<forwardIndex*()> fidxInRamConstructor, bool deleteFirst= false, sequentialConstructions *consQueue= NULL ) : slowCons_(&fidx, fidxInRamConstructor, deleteFirst, consQueue) {
        }
        
        inline void
            getWordsRegs( uint32_t docID, std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const {
                slowCons_.getObject()->getWordsRegs(docID, words, regions);
            }
        
        inline uint32_t
            numDocs() const { return slowCons_.getObject()->numDocs(); }
    
    private:
        
        slowConstruction<forwardIndex const> slowCons_;
};

#endif
