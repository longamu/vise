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

#include <iostream>
#include <string.h>

int main(){
    /*
    {
    std::cout<<"oxc1_5k\n";
    documentMap docMap( "/home/relja/Relja/Data/oxc1_5k/dset_oxc1_5k.db", "oxc1_5k" );
    docMap.saveToFile( "/home/relja/Relja/Data/oxc1_5k/docMap_oxc1_5k.bin" );
    }*/
    
    std::cout<<"ox100k\n";
//     documentMap docMap( "/home/relja/Relja/Data/ox100k/dset_ox100k.db", "ox100k" );
//     docMap.saveToFile( "/home/relja/Relja/Data/ox100k/docMap_ox100k.bin" );
    documentMap docMap( "/home/relja/Relja/Data/ox100k/docMap_ox100k.bin" );
    
    std::cout<< docMap.h2i( "0027ac9920a67f66f783ecddbaa54ad4fe380f024f82e1fe916b4199d4d240f0" ) <<"\n";
    std::cout<< docMap.h2i( std::string("0027ac9920a67f66f783ecddbaa54ad4fe380f024f82e1fe916b4199d4d240f0") ) <<"\n";
    std::cout<< docMap.i2h( 1632-1 ) <<"\n";
    
    return 0;
    
}
