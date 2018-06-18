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

#ifndef _DATASET_H_
#define _DATASET_H_

#include <stdint.h>
#include <string>

#include <sqlite3.h>

class dataset {
    
    public:
        
        dataset( const char fileName[], const char prefix[], std::string aFindPath= "", std::string aReplacePath= "" );
        
        ~dataset();
        
        uint32_t
            getNumDoc();
        
        uint32_t
            getDocID( const char hash[] );
        
        std::string
            getHash( uint32_t docID );
        
        std::string
            getFn( uint32_t docID );
        
        std::pair<uint32_t, uint32_t>
            getWidthHeight( uint32_t docID );
    
    private:
        
        sqlite3 *db;
        const char *prefix;
        std::string findPath, replacePath;
    
};

#endif
