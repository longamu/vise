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

#ifndef _RELJA_PUTATIVE_H_
#define _RELJA_PUTATIVE_H_

#include <vcl_vector.h>
#include <vcl_utility.h>
#include <vcl_algorithm.h>

#include <stdint.h>
#include <limits>
#include <assert.h>

#include "jp_dist2.hpp"



class putative_quantized {
    
    public:
        
        static void
            getPutativeMatches(
                    vcl_vector<uint32_t> &ids1, vcl_vector<uint32_t> &ids2,
                    vcl_vector< vcl_pair<uint32_t, uint32_t> > &putativeMatches );
    
};



template<class DescType>
class putative_desc {
    
    public:
        
        static void
            getPutativeMatches(
                    const DescType *desc1, uint32_t size1, const DescType *desc2, uint32_t size2, uint32_t nDims, vcl_vector< vcl_pair<uint32_t, uint32_t> > &putativeMatches, bool useLowe= true, float deltaSq= 0.81f, float epsilon= 100.0f );
                    
};


class indSorter {
        
    private:
        
        vcl_vector<uint32_t> *toSort;
    
    public:
        
        indSorter( vcl_vector<uint32_t> &aToSort )
            { toSort= &aToSort; }
        
        ~indSorter()
            { toSort= NULL; }
        
        static void
            sort( vcl_vector<uint32_t> &aToSort, vcl_vector<uint32_t> &inds );
        
        inline int
            operator()( uint32_t leftInd, uint32_t rightInd ){
                // careful here because has to be strictly < and not <=, otherwise sometimes causes segmentation fault!
                if ( (*toSort)[ leftInd ] < (*toSort)[ rightInd ] )
                    return true;
                else 
                    return false;
            }
            
};










template<class DescType>
void
putative_desc<DescType>::getPutativeMatches(
    const DescType *desc1, uint32_t size1, const DescType *desc2, uint32_t size2, uint32_t nDims, vcl_vector< vcl_pair<uint32_t, uint32_t> > &putativeMatches, bool useLowe, float deltaSq, float epsilon ) {
    
    float dsq;
    
    if (!useLowe) { // Use the epsilon measure.
        
        for (uint32_t i=0; i<size1 ; ++i) {
            for (uint32_t j=0; j<size2 ; ++j) {
                dsq = jp_dist_l2(&desc1[i*nDims], &desc2[j*nDims], nDims);
                if (dsq < (epsilon*epsilon))
                    putativeMatches.push_back(std::make_pair(i, j));
            }
        }
      
    } else {
        
        static const int numnn = 2;
        std::pair<uint32_t,float> nns[numnn+1];
        int k;
        
        for (uint32_t i=0; i<size1; ++i) {
            
            nns[0] = nns[1] = nns[2] = std::make_pair(-1, std::numeric_limits<float>::max());
            for (uint32_t j=0; j<size2; ++j) {
                
                // One potential improvement is to copy desc1 into an aligned
                // buffer...
                dsq = jp_dist_l2(&desc1[i*nDims], &desc2[j*nDims], nDims);
                
                if (dsq < nns[numnn-1].second) {
                    for (k=numnn; k>0 && nns[k-1].second > dsq; --k)
                        nns[k] = nns[k-1];
                    nns[k] = std::make_pair(j, dsq);
                }
                
            }
            assert(nns[0].second <= nns[1].second);
            if ((nns[0].second/nns[1].second) < deltaSq) {
                putativeMatches.push_back(std::make_pair(i, nns[0].first));
            }
            
        }
        
    }
    
}




#endif //_RELJA_PUTATIVE_H_
