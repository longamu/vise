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

#ifndef _IIDX_WITH_DATA_BUILDER_H_
#define _IIDX_WITH_DATA_BUILDER_H_


#include <stdint.h>
#include <string>

#include "index_with_data.h"
#include "compressor.h"


class iidxWithDataBuilder {
    
    public:
        
        static void
            build( indexWithData const &fidx, std::string fileName, compressorIndep const &comp, std::string tmpDir= "" );
    
    private:
        
        static void
            buildInter( indexWithData const &fidx, std::string fileName, std::string tmpDir= "" );
    
};

#endif
