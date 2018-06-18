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

#include <vector>
#include <iostream>

#include "ellipse.h"
#include "feat_getter.h"
#include "feat_standard.h"


int main(int argc, char **argv) {
    
    featGetter *featGetterObj= new featGetter_standard( "hesaff-sift" );
//     featGetter *featGetterObj= new featGetter_standard( "sande-opponentsift" );
    
    uint32_t numFeats;
    std::vector<ellipse> regions;
    float *descs;
    
    featGetterObj->getFeats( argv[1], numFeats, regions, descs );
    
//     featGetterObj->getFeats( argv[1], 100, 300, 200, 400, numFeats, regions, descs );
    
    std::cout<<"extracted "<<numFeats<<"\n";
    
    double x,y,a,b,c;
    uint32_t const numDims= featGetterObj->numDims();
    
    for (uint32_t i=0; i<2 && i<numFeats; ++i){
        regions[i].get(x,y,a,b,c);
        std::cout<<x<<" "<<y<<" "<<a<<" "<<b<<" "<<c<<": ";
        for (uint32_t j=0; j<numDims; ++j)
            std::cout<<static_cast<int>(descs[i*numDims+j])<<" ";
        std::cout<<"\n";
    }
    
    delete []descs;
    delete featGetterObj;
    
    return 0;
    
}
