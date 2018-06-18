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

#ifndef _IMAGE_GRAPH_H_
#define _IMAGE_GRAPH_H_

#include <map>
#include <set>

#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>

#include "spatial_verif.h"


class imageGraph {
    
    public:
        
        typedef std::map<uint32_t, std::vector<spatResType> > imageGraphType;
        
        // load
        imageGraph( const char fileName[], std::set<uint32_t> *excludes= NULL ){ loadFromFile(fileName); removeExcludes(excludes); }
        
        // compute
        imageGraph( const char fileName[], uint32_t numDocs, spatialVerif &spatialVerif_obj, uint32_t maxNeighs= 0, double scoreThr= static_cast<double>(spatParams_def.minInliers), std::set<uint32_t> *excludes= NULL );
        
        void
            makeSymmetric();
        
        void
            computeInverse( imageGraphType &invGraph );
        
        // careful: this can make the graph non-symmetric!
        void
            keepVerified( uint32_t maxNeighs= 50, double scoreThr= 15.0, bool adaptiveThr= true, uint32_t adaptiveMinVerif= 3, bool forceSymmetric= false );
        
        void
            removeExcludes( std::set<uint32_t> *excludes);

        void
            saveToFile( const char filename[] );
        
        void
            loadFromFile( const char filename[] );
        
        imageGraphType graph;
    
    private:
        
        friend class boost::serialization::access;
        template<class A>
        void serialize(A & archive, const unsigned int version){
            archive & graph;
        }
        
};

#endif
