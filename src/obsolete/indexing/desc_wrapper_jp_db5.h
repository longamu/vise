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

#ifndef _FEAT_WRAPPER_JP_DB5_H_
#define _FEAT_WRAPPER_JP_DB5_H_

#include "desc_getter_from_file.h"
#include "jp_db5.hpp"
#include "document_map.h"
#include "macros.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>



template<class descT>
class descWrapperJpDb5 : public descGetterFromFile {
    
    public:
        
        descWrapperJpDb5( const char fileName[], uint32_t aNumDims, documentMap *aDocMap= NULL ) : jp_db5_obj(fileName,"r"), docMap(aDocMap), numDims_(aNumDims), numDocs_(jp_db5_obj.size()/2)
            {}
        
        virtual ~descWrapperJpDb5(){ jp_db5_obj.close(); }
        
        void
            getDescs( uint32_t docID, uint32_t &numDescs, float **&descs ) const;
        
        void
            getDescs( const char id[], uint32_t &numDescs, float **&descs ) const;
        
        uint32_t
            numDims() const { return numDims_; }
        
        virtual uint32_t
            numDocs() const { return numDocs_; }
    
    private:
        
        jp_db5 jp_db5_obj;
        documentMap *docMap;
        uint32_t numDims_, numDocs_;
    
};



template<class descT>
class descHelper {};

template<>
class descHelper<uint8_t> {
    public:
        static inline uint32_t dtypeStrLen(){ return 5; } // dtype_str== "'|u1'"
        static void convertToFloat( uint8_t **&descs_, uint32_t numDescs, uint32_t numDims, float **&descs ){
            descs= new float*[numDescs];
            for (uint32_t iF=0; iF<numDescs; ++iF){
                descs[iF]= new float[numDims];
                for (uint32_t iD=0; iD<numDims; ++iD)
                    descs[iF][iD]= static_cast<float>( descs_[iF][iD] );
                delete []descs_[iF];
            }
            delete []descs_;
        }
};

template<>
class descHelper<float> {
    public:
        static inline uint32_t dtypeStrLen(){ return 5; } // dtype_str== "'|f4'"
        static inline void convertToFloat( float **&descs_, uint32_t numDescs, uint32_t numDims, float **&descs ){ descs= descs_; }
};



template<class descT>
void
descWrapperJpDb5<descT>::getDescs( const char id[], uint32_t &numDescs, float **&descs ) const {
    
    char *hash_d= NULL;
    uint32_t lenHash= strlen(id);
    
    hash_d= new char[lenHash+2+1];
    memcpy(hash_d, id, lenHash*sizeof(char));
    hash_d[lenHash]  ='_';
    hash_d[lenHash+1]='d';
    hash_d[lenHash+2]='\0';
    
    char *p;
    uint32_t sz;
    
    jp_db5_obj.get( hash_d, &p, &sz );
    delete []hash_d;
    
    //-------- read the data
    
    // read the dtype
    
    uint32_t dtypelen_off, dtypestr_off, dtypelen;
    memcpy(&dtypelen_off, p + sz - sizeof(uint32_t), sizeof(uint32_t));
    ASSERT( dtypelen_off < sz );
    memcpy(&dtypelen, p + dtypelen_off, sizeof(uint32_t));
    dtypestr_off= dtypelen_off + sizeof(uint32_t);
    if (dtypelen!= descHelper<descT>::dtypeStrLen() ) {
        ASSERT(dtypestr_off + dtypelen <= sz );
        char *dtype_str= (char*)malloc(dtypelen + 1);
        ASSERT(dtype_str);
        dtype_str[dtypelen] = 0; //'\0'
        memcpy(dtype_str, p + dtypestr_off, dtypelen*sizeof(char));
        std::cerr<<"Don't know what data is this!? dtype_str="<<dtype_str<<"\n";
        free(dtype_str);
        throw std::runtime_error( "Don't know what data is this!?" );
    }
    
    // read the shape
    
    uint32_t ndlen_off, nddata_off, ndlen;
    ndlen_off= dtypestr_off + dtypelen*sizeof(char);
    nddata_off= ndlen_off + sizeof(uint32_t);
    memcpy(&ndlen, p + ndlen_off, sizeof(uint32_t));
    ASSERT( (nddata_off + ndlen*sizeof(uint32_t))<=sz );
    
    ASSERT(ndlen==2);
    memcpy(&numDescs, p + nddata_off, sizeof(uint32_t));
    uint32_t thisNumDims;
    memcpy(&thisNumDims, p + nddata_off + sizeof(uint32_t), sizeof(uint32_t));
    ASSERT(numDims_==thisNumDims);
    
    // allocate memory and read data
    
    descT **descs_= new descT*[numDescs]; // don't delete - managed in converToFloat
    for (uint32_t iD=0; iD<numDescs; ++iD){
        descs_[iD]= new descT[numDims_];
        memcpy( descs_[iD], p + iD*numDims_*sizeof(descT), numDims_*sizeof(descT) );
    }
    
    // convert to float
    descHelper<descT>::convertToFloat( descs_, numDescs, numDims_, descs );
    
    // cleanup
    free(p);
    
}



template<class descT>
void
descWrapperJpDb5<descT>::getDescs( uint32_t docID, uint32_t &numDescs, float **&descs ) const {
    getDescs( docMap->i2h(docID).c_str(), numDescs, descs );
}



#endif
