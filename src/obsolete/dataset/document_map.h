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

#ifndef _DOCUMENT_MAP_H_
#define _DOCUMENT_MAP_H_

#include <stdint.h>
#include <string>
#include <map>
#include <iostream>
#include <fstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/map.hpp>

#include "dataset.h"
#include "dataset_abs.h"
#include "macros.h"



class documentMap {
    
    public:
        
        documentMap( dataset *aDSet ) : dsetCreatedHere(false) {
            dset= aDSet;
            hasDSet= (dset!=NULL);
        }
        
        documentMap( const char dsetFileName[], const char prefix[], std::string findPath= "", std::string replacePath= "" );
        
        documentMap( const char dmapFileName[], dataset *aDSet= NULL ){
            loadFromFile(dmapFileName, aDSet);
        }
        
        ~documentMap(){
            if (dsetCreatedHere)
                delete dset;
        }
        
        inline uint32_t
            h2i( const char hash[] ) const { return h2i( std::string(hash) ); }
        
        inline uint32_t
            h2i( std::string hash ) const {return h2i_map.find(hash)->second; }
        
        inline std::string
            i2h( uint32_t docID ) const { return i2h_map.find(docID)->second; }
        
        inline std::string
            getFn( uint32_t docID ) const {
                if (hasDSet)
                    return dset->getFn(docID);
                else
                    throw std::runtime_error("no dataset file provided");
            }
        
        inline std::pair<uint32_t, uint32_t>
            getWidthHeight( uint32_t docID ) const {
                if (hasDSet)
                    return dset->getWidthHeight(docID);
                else
                    throw std::runtime_error("no dataset file provided");
            }
        
        inline uint32_t
            numDocs() const { return numDocs_; }
        
        void
            saveToFile( const char fileName[] ) const;
        
        void
            loadFromFile( const char fileName[], dataset *aDSet= NULL );
        
        
    private:
        
        friend class boost::serialization::access;
        template<class A>
        void save(A & archive, const unsigned int version) const;
        template<class A>
        void load(A & archive, const unsigned int version);
        BOOST_SERIALIZATION_SPLIT_MEMBER()
        
        dataset *dset;
        bool dsetCreatedHere;
        uint32_t numDocs_;
        bool hasDSet;
        
        std::map< std::string, uint32_t > h2i_map;
        std::map< uint32_t, std::string > i2h_map;
        
        DISALLOW_COPY_AND_ASSIGN(documentMap)
    
};



class documentMapAsDatasetAbs : public datasetAbs {
    public:
        documentMapAsDatasetAbs(documentMap const &docMap) : docMap_(&docMap) {}
        virtual ~documentMapAsDatasetAbs(){}
        
        inline uint32_t
            getNumDoc() const { return docMap_->numDocs(); }
        
        inline std::string
            getFn( uint32_t docID ) const { return docMap_->getFn(docID); }
        
        virtual std::pair<uint32_t, uint32_t>
            getWidthHeight( uint32_t docID ) const { return docMap_->getWidthHeight(docID); }
        
        inline uint32_t
            getDocID( std::string fn ) const { throw std::runtime_error("Not implemented"); }
        
        inline uint32_t
            getDocIDFromAbsFn( std::string fn ) const { throw std::runtime_error("Not implemented"); }
        
        inline bool
            containsFn( std::string fn ) const { throw std::runtime_error("Not implemented"); }
        
    private:
        documentMap const *docMap_;
        DISALLOW_COPY_AND_ASSIGN(documentMapAsDatasetAbs)
};

#endif
