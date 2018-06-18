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

#include "putative.h"

#include <vcl_vector.h>
#include <vcl_utility.h>
#include <vcl_algorithm.h>

#include <stdint.h>


void
indSorter::sort( vcl_vector<uint32_t> &aToSort, vcl_vector<uint32_t> &inds ){
    
    indSorter indSorterObj= indSorter( aToSort );
    
    inds.clear();
    inds.reserve( aToSort.size() );
    for (uint32_t i=0; i < aToSort.size(); ++i)
        inds.push_back( i );
    
    vcl_sort( inds.begin(), inds.end(), indSorterObj );
    
}


void
putative_quantized::getPutativeMatches(
        vcl_vector<uint32_t> &ids1, vcl_vector<uint32_t> &ids2,
        vcl_vector< vcl_pair<uint32_t, uint32_t> > &putativeMatches ){
    
    vcl_vector<uint32_t> inds1, inds2;
    indSorter::sort( ids1, inds1 );
    indSorter::sort( ids2, inds2 );
    
    putativeMatches.clear();
    
    uint32_t i1=0, i2=0, i2temp;
    
    while ( i1 < inds1.size() && i2 < inds2.size() ){
        if   (ids1[ inds1[i1] ] < ids2[ inds2[i2] ])
            ++i1;
        else if (ids1[ inds1[i1] ] > ids2[ inds2[i2] ])
            ++i2;
        else {
            // equal
            i2temp= i2;
            while ( i2temp < inds2.size() && ids1[ inds1[i1] ] == ids2[ inds2[i2temp] ] ){
                putativeMatches.push_back( vcl_make_pair(inds1[i1], inds2[i2temp]) );
                ++i2temp;
            }
            ++i1;
        }
    }
    
}
