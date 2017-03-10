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

#ifndef _REPRESENT_H_
#define _REPRESENT_H_

#include <stdint.h>
#include <vector>
#include <map>

#include "forward_index.h"
#include "ellipse.h"
#include "query.h"
#include "homography.h"



class represent {
    
    public:
        
        represent() : doDelQuery(false) {}
        
        represent(represent const &represent_obj) : BoW(represent_obj.BoW), words(represent_obj.words), regions(represent_obj.regions), doDelQuery(true) {
            query_obj= new query( *(represent_obj.query_obj) );
        }
        
        represent( forwardIndex const &forwardIndex_obj, query const &aQueryIn_obj, bool doComputeBow= true ){
            query_obj= &aQueryIn_obj;
            doDelQuery= false;
            load( forwardIndex_obj, doComputeBow );
        }
        
        represent( forwardIndex const &forwardIndex_obj, uint32_t docID, bool doComputeBow= true ){
            query_obj= new query(docID, true);
            doDelQuery= true;
            load( forwardIndex_obj, doComputeBow );
        }
        
        // careful - changes arguments!
        represent( query const &aQueryIn_obj, std::vector<quantDesc> &aWords, std::vector<ellipse> &aRegions, bool aDoDelQuery= false ) : doDelQuery(aDoDelQuery) {
            query_obj= &aQueryIn_obj;
            words.swap(aWords);
            regions.swap(aRegions);
            filterROI();
            computeBoW( words, BoW );
        }
        
        ~represent() {
            if (doDelQuery)
                delete query_obj;
        }
        
        inline bool
            isInternal() const { return query_obj->isInternal; }
        
        void
            load( forwardIndex const &forwardIndex_obj, bool doComputeBow= true );
        
        void
            filterROI();
        
        void
            add( forwardIndex const &forwardIndex_obj, uint32_t docID, homography *H= NULL, bool addBoW= true );
        
        static void
            weight( std::map<uint32_t,double> &aBoW, std::vector<double> const &aWordWeight );
        
        static void
            computeBoW( std::vector<quantDesc> &aWords, std::map<uint32_t,double> &aBoW, std::vector<quantDesc>::iterator *begin= NULL );
        
        static void
            l2Normalize( std::map<uint32_t,double> &aBoW );
        
            
        std::map<uint32_t,double> BoW;
        std::vector<quantDesc> words;
        std::vector<ellipse> regions;
        
        bool doDelQuery;
        query const *query_obj;
        
};

#endif
