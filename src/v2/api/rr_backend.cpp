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

#include "api_v2.h"

int main(int argc, char* argv[]){
    MPI_INIT_ENV
    if ( argc != 4 ) {
      std::cout << "\nUsage: " << argv[0] << " backend-port dataset-name config-file" << std::endl;
      return 0;
    }

    std::vector< std::string > param;
    for( int i=0; i<argc; i++) {
      param.push_back( argv[i] );
    }
    api_v2( param );
    return 0;
}

