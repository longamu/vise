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

#include "document_map.h"



documentMap::documentMap( const char dsetFileName[], const char prefix[], std::string findPath, std::string replacePath ) : dsetCreatedHere(true), hasDSet(true) {
    
    dset= new dataset( dsetFileName, prefix, findPath, replacePath );
    numDocs_= dset->getNumDoc();
    
    for (uint32_t docID=0; docID<numDocs_; ++docID){
        std::string hash= dset->getHash( docID );
        if (h2i_map.count( hash )){
            std::cout<<"Hash keys should be unique!\n"; // throw exception
        } else {
            h2i_map[ hash ]= docID;
            i2h_map[ docID ]= hash;
        }
    }
        
}



void
documentMap::saveToFile( const char fileName[] ) const {
    
    std::ofstream ofs(fileName, std::ios::binary);
    boost::archive::binary_oarchive oa(ofs);
    oa << (*this);
    
}



void
documentMap::loadFromFile( const char fileName[], dataset *aDSet ){
    
    std::ifstream ifs(fileName, std::ios::binary);
    boost::archive::binary_iarchive ia(ifs);
    ia >> (*this);
    
    dset= aDSet;
    hasDSet= (dset!=NULL);
    
}



template<class A>
void
documentMap::save(A & archive, const unsigned int version) const {
    archive & numDocs_ & h2i_map & i2h_map;
}



template<class A>
void
documentMap::load(A & archive, const unsigned int version){
    dset= NULL; dsetCreatedHere= false; hasDSet= false;
    archive & numDocs_ & h2i_map & i2h_map;
}
