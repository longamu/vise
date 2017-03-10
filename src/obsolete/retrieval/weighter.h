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

#ifndef _WEIGHTER_H_
#define _WEIGHTER_H_

#include <stdint.h>
#include <vector>
#include <map>

#include "tfidf.h"
#include "inverted_index.h"

class tfidf;

class weighter {
    
    public:
        
        static void
            queryExecute( invertedIndex const *aIidx, std::map<uint32_t,double> const &wordWeight, std::vector<double> const &aIdf, std::vector<double> const &aDocL2, std::vector<double> &scores, double defaultScore= 0.0 );
        
        static void
            queryExecute( std::map<uint32_t,double> const &wordWeight, tfidf const *tfidf_obj, std::vector<double> &scores, double defaultScore= 0.0 );
    
};


#endif
