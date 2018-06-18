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

#include "fidx_wrapper_jp_db5.h"

#include <stdlib.h>
#include <stdexcept>
#include <cstring>
#include <string>
#include <iostream>

#include <boost/thread.hpp>

#include "util.h"
#include "macros.h"



fidxWrapperJpDb5::fidxWrapperJpDb5( const char fileName[], documentMap &aDocMap, const char mode[] ) :
        docMap(&aDocMap), jp_db5_obj(fileName, mode) {
    numDocs_= docMap->numDocs();
    // read the entire file - helps with loading very large files
    boost::thread t(boost::bind(util::visitFile,fileName));
    t.detach();
}



void
fidxWrapperJpDb5::getWordsRegs( uint32_t docID, std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const {
    
    char *data;
    uint32_t size;
    
    std::string hash= docMap->i2h(docID);
    
    try {
        jp_db5_obj.get( hash.c_str(), &data, &size );
        decompose_data( data, size, words, regions );
        free(data);
    }
    catch (std::runtime_error) {
        //TODO should be KeyError, check this
        // KeyError: document might be featureless
        words.clear();
        regions.clear();
    }
    
}



void
fidxWrapperJpDb5::setWordsRegs( uint32_t docID, std::vector<quantDesc> const &words, std::vector<ellipse> const &regions ){
    
    char *data;
    uint32_t size;
    
    std::string hash= docMap->i2h(docID); 
    
    compose_data( data, size, words, regions );
    jp_db5_obj.set( hash.c_str(), data, size, JP_DB5_GZIP, 1 ); // compression=0-9, 0=no
    
    free(data);
    
}



// ----------- fidxWrapperJpDb5_HardJP

// adapted from jp_db5.pyx
void
fidxWrapperJpDb5_HardJP::decompose_data(
    char *p, uint32_t sz, 
    std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const {
    
    words.clear();
    regions.clear();
    
    uint32_t i;
    uint32_t dtypelen, ndlen;
    char *dtype_str;
    uint32_t dtypelen_off, dtypestr_off, ndlen_off, nddata_off, numFeatures;
    bool hasTheta= true;
    //  object shape, dtype;
    
    if ( sz<3*sizeof(uint32_t) ) // Something must be wrong here.
        throw std::runtime_error( "Array size is too small!" );
    
    // read the dtype
    memcpy(&dtypelen_off, p + sz - sizeof(uint32_t), sizeof(uint32_t));
    ASSERT( dtypelen_off < sz );
    memcpy(&dtypelen, p + dtypelen_off, sizeof(uint32_t));
    dtypestr_off = dtypelen_off + sizeof(uint32_t);
    ASSERT(dtypestr_off + dtypelen <= sz );
    dtype_str = (char*)malloc(dtypelen + 1);
    ASSERT(dtype_str);
    dtype_str[dtypelen] = 0; //'\0'
    // dtype_str== "[('id', '<u4'), ('x', '<f4'), ('y', '<f4'), ('a', '<f4'), ('b', '<f4'), ('c', '<f4'), ('theta', '<f4')]";
    // strlen( dtype_str )=103
    // or without theta: 85
    if (dtypelen==103){
        hasTheta= true;
    } else if (dtypelen==85){
        hasTheta= false;
    } else {
        memcpy(dtype_str, p + dtypestr_off, dtypelen*sizeof(char));
        std::cerr<<"Don't know what data is this!? dtype_str="<<dtype_str<<"\n";
        free(dtype_str);
        throw std::runtime_error( "Don't know what data is this!?" );
    }
    
    // read the shape
    ndlen_off = dtypestr_off + dtypelen*sizeof(char);
    nddata_off = ndlen_off + sizeof(uint32_t);
    memcpy(&ndlen, p + ndlen_off, sizeof(uint32_t));
    ASSERT( (nddata_off + ndlen*sizeof(uint32_t))<=sz );
    
    ASSERT(ndlen==1);
    memcpy(&numFeatures, p + nddata_off, sizeof(uint32_t));
    
    // read the data
    words.reserve(numFeatures);
    regions.reserve(numFeatures);
    
    int stride= 6 + int(hasTheta); // number of fields
    int id_off= 0, x_off= 4, y_off =8, a_off =12, b_off =16, c_off =20;
    float *x, *y, *a, *b, *c;
    uint32_t *id;
    id= (uint32_t*)&p[id_off];
    x= (float*)&p[x_off];
    y= (float*)&p[y_off];
    a= (float*)&p[a_off];
    b= (float*)&p[b_off];
    c= (float*)&p[c_off];
    
    for (i=0; i<numFeatures; ++i){
        words.push_back( quantDesc(id[stride*i]) );
        regions.push_back( ellipse( x[stride*i],y[stride*i],a[stride*i],b[stride*i],c[stride*i] ) );
    }
    
    free(dtype_str);
    
}


// ----------- fidxWrapperJpDb5_FixK


void
fidxWrapperJpDb5_FixK::compose_data(
    char *&p, uint32_t &sz, 
    std::vector<quantDesc> const &words, std::vector<ellipse> const &regions ) {
    
    uint32_t len= words.size();
    
    ASSERT( len == regions.size() );
    
    sz= sizeof(uint32_t) // len
      + len* ( K* ( sizeof(uint32_t) // word IDs
                    + (hasWeights?sizeof(float):0) ) // word weights
               + ellipse::getSize() // ellipse information
               );
    
    p= (char*)malloc(sz);
    char *p_= p;
    
    memcpy( p_, &len, sizeof(uint32_t) );
    p_+= sizeof(uint32_t);
    uint iNN;
    
    if (hasWeights) {
        
        for (std::vector<quantDesc>::const_iterator itQD= words.begin(); itQD!=words.end(); ++itQD){
            for (iNN=0; iNN<K; ++iNN){
                memcpy( p_ , &(itQD->rep[iNN].first), sizeof(uint32_t) );
                p_+= sizeof(uint32_t);
                memcpy( p_ , &(itQD->rep[iNN].second), sizeof(float) );
                p_+= sizeof(float);
            }
        }
        
    } else {
        
        for (std::vector<quantDesc>::const_iterator itQD= words.begin(); itQD!=words.end(); ++itQD){
            memcpy( p_ , &(itQD->rep[0].first), sizeof(uint32_t) );
            p_+= sizeof(uint32_t);
        }
        
    }
    
    for (std::vector<ellipse>::const_iterator itR= regions.begin(); itR!=regions.end(); ++itR)
        itR->setMem( p_ );
    
}



void
fidxWrapperJpDb5_FixK::decompose_data(
    char *p, uint32_t sz, 
    std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const {
    
    words.clear();
    regions.clear();
    
    char *p_= p;
    
    uint32_t len;
    memcpy( &len, p_, sizeof(uint32_t) );
    p_+= sizeof(uint32_t);
    
    words.resize(len);
    regions.resize(len);
    
    
    uint32_t wordID;
    float weight;
    
    if (hasWeights){
        
        float totalW;
        
        for (uint32_t iReg= 0; iReg<len; ++iReg){
            totalW= 0.0f;
            for (uint iNN= 0; iNN<K; ++iNN){
                memcpy(&wordID, p_, sizeof(uint32_t));
                memcpy(&weight, p_+sizeof(uint32_t), sizeof(float));
                p_+= sizeof(uint32_t)+sizeof(float);
                words[iReg].rep.push_back( std::make_pair(wordID, weight) );
                totalW+= weight;
            }
            if (doNormalize){
                for (uint iNN= 0; iNN<K; ++iNN)
                    words[iReg].rep[iNN].second/= totalW;
            }
        }
        
    } else {
        
        for (uint32_t iReg= 0; iReg<len; ++iReg){
            memcpy(&wordID, p_, sizeof(uint32_t));
            p_+= sizeof(uint32_t);
            words[iReg].rep.push_back( std::make_pair(wordID, 1.0f) );
        }
        
    }
    
    for (uint32_t iReg= 0; iReg<len; ++iReg){
        regions[iReg].setMem(p_);
    }
    
}



void
fidxWrapperJpDb5_FixK::convertFromHardJP( fidxWrapperJpDb5_HardJP &fidxFrom, const char fileName[], uint K ){
    
    std::cout<<"fidxWrapperJpDb5_FixK: converting from JP's hard assignment\n";
    
    ASSERT( K==0 || K==1 );
    
    std::vector<quantDesc> words;
    std::vector<ellipse> regions;
    
    {
    
    fidxWrapperJpDb5_FixK fidxTo( fileName, *(fidxFrom.docMap), K, "w" );
    
    uint32_t numDocs= fidxFrom.numDocs();
    for (uint32_t docID= 0; docID<numDocs; ++docID){
        if (docID%1000==0)
            std::cout<<"fidxWrapperJpDb5_FixK: processing docID= "<<docID<<"\n";
        fidxFrom.getWordsRegs( docID, words, regions );
        fidxTo.setWordsRegs( docID, words, regions );
    }
    
    }
    
}



void
fidxWrapperJpDb5_FixK::computeWords( documentMap &aDocMap, const char wordFn[], descGetterFromFile *desc_obj, const char featFn[], const char asgnFn[], uint KNN, softAssigner *SA_obj ){
    
    std::cout<<"fidxWrapperJpDb5_FixK: computing words\n";
    
    jp_db5 feat_obj(featFn, "r");
    jp_db5 asgn_obj(asgnFn, "r");
    
    if (KNN>1) ASSERT(SA_obj!=NULL);
    bool needsFeat= (KNN>1)?SA_obj->needsFeat():false;
    if (needsFeat)
        ASSERT(desc_obj!=NULL);
    
    fidxWrapperJpDb5_FixK fidxTo( wordFn, aDocMap, KNN, "w" );
    
    void *itA= asgn_obj.get_iterator();
    
    const char *hash_d= NULL;
    char *hash_r= NULL, *hash= NULL;
    uint32_t lenHash_d;
    float *feats= NULL;
    
    char *asgnData, *regionData;
    uint32_t asgnSize, regionSize, numFeats;
    
    // no need for docID, can remove it later for slight speedup and cleaner code
    uint32_t docID;
    
    uint32_t progress_= 0;
    
    std::vector<quantDesc> words;
    std::vector<ellipse> regions;
    
    for(; !jp_db5_iterator_finished(itA); jp_db5_iterator_advance(itA)){
        
        if (progress_%1000==0)
            std::cout<<"fidxWrapperJpDb5_FixK: num processed= "<<progress_<<"\n";
        ++progress_;
        
        // hash_d= <random>_d
        hash_d= jp_db5_iterator_key(itA);
        lenHash_d= strlen(hash_d);
        hash= new char[lenHash_d-1];
        memcpy( hash, hash_d, (lenHash_d-2)*sizeof(char) ); hash[lenHash_d-2]='\0';
        hash_r= new char[lenHash_d+1];
        memcpy( hash_r, hash_d, lenHash_d*sizeof(char) ); hash_r[lenHash_d-1]='r'; hash_r[lenHash_d]='\0';
        
        if (needsFeat)
            desc_obj->getDescs( aDocMap.h2i(hash), numFeats, feats );
        asgn_obj.get( hash_d, &asgnData, &asgnSize );
        fidxTo.decomposeAsgn( asgnData, asgnSize, KNN, words, SA_obj, feats );
        if (needsFeat)
            delete []feats;
        free(asgnData);
        
        // hash_r= <random>_r
        feat_obj.get( hash_r, &regionData, &regionSize );
        delete []hash_r;
        decomposeReg( regionData, regionSize, regions );
        free(regionData);
        
        // save
        docID= aDocMap.h2i( hash );
        fidxTo.setWordsRegs( docID, words, regions );
        
        delete []hash;
        
    }
    
    jp_db5_del_iterator( itA );
    
    asgn_obj.close();
    feat_obj.close();
    
    std::cout<<"fidxWrapperJpDb5_FixK: computing words - DONE\n";
    
}



void
fidxWrapperJpDb5_FixK::decomposeAsgn( char *p, uint32_t sz, uint KNN, std::vector<quantDesc> &words, softAssigner *SA_obj, float *feats ) const {
    
    words.clear();
    KNN= (KNN>0)?KNN:1;
    bool needsFeat= (KNN>1)?SA_obj->needsFeat():false;
    if (needsFeat) ASSERT(feats!=NULL);
    
    if ( sz<3*sizeof(uint32_t) ) // Something must be wrong here.
        throw std::runtime_error( "Array size is too small!" );
    
    uint32_t dtypelen, ndlen;
    char *dtype_str;
    uint32_t dtypelen_off, dtypestr_off, ndlen_off, nddata_off, numFeatures;
    
    // read the dtype
    memcpy(&dtypelen_off, p + sz - sizeof(uint32_t), sizeof(uint32_t));
    ASSERT( dtypelen_off < sz );
    memcpy(&dtypelen, p + dtypelen_off, sizeof(uint32_t));
    dtypestr_off = dtypelen_off + sizeof(uint32_t);
    ASSERT(dtypestr_off + dtypelen <= sz );
    dtype_str = (char*)malloc(dtypelen + 1);
    ASSERT(dtype_str);
    dtype_str[dtypelen] = 0; //'\0'
    // dtype_str== "[('inds', '<u4'), ('dsqs', '<f4')]"
    // strlen(dtype_str)== 34
    if (dtypelen!=34) {
        memcpy(dtype_str, p + dtypestr_off, dtypelen*sizeof(char));
        std::cerr<<"Don't know what data is this!? dtype_str="<<dtype_str<<"\n";
        free(dtype_str);
        throw std::runtime_error( "Don't know what data is this!?" );
    }
    
    // read the shape
    ndlen_off = dtypestr_off + dtypelen*sizeof(char);
    nddata_off = ndlen_off + sizeof(uint32_t);
    memcpy(&ndlen, p + ndlen_off, sizeof(uint32_t));
    ASSERT( (nddata_off + ndlen*sizeof(uint32_t))<=sz );
    
    ASSERT(ndlen==2);
    memcpy(&numFeatures, p + nddata_off, sizeof(uint32_t));
    uint32_t assgnKNN;
    memcpy(&assgnKNN, p + nddata_off + sizeof(uint32_t), sizeof(uint32_t));
    ASSERT(assgnKNN>=KNN);
    
    // read the data
    words.reserve(numFeatures);
    
    uint32_t stride= sizeof(uint32_t)+sizeof(float); // size of fields
    uint32_t i;
    uint32_t wordID;
    float distSq;
    
    for (i=0; i<numFeatures; ++i){
        
        quantDesc ww;
        
        for (uint iNN=0; iNN < KNN; ++iNN){
            
            wordID= *((uint32_t*)(p +iNN*stride));
            distSq= *((float*)(p+sizeof(uint32_t) +iNN*stride));
            
            ww.rep.push_back( std::make_pair(wordID, distSq) );
        }
        
        p+= assgnKNN*stride;
        
        if (KNN>1){
            if (!needsFeat)
                SA_obj->getWeights(ww);
            else {
                ASSERT(0); // fix this if ever needed and change 0 to numDims in line below
                SA_obj->getWeights(ww, feats+i*0);
            }
        }
        else
            ww.rep[0].second= 1.0f;
        
        words.push_back(ww);
        
    }
    
    free(dtype_str);
        
}



void
fidxWrapperJpDb5_FixK::decomposeReg( char *p, uint32_t sz, std::vector<ellipse> &regions ) {
    
    regions.clear();
    
    if ( sz<3*sizeof(uint32_t) ) // Something must be wrong here.
        throw std::runtime_error( "Array size is too small!" );
    
    uint32_t dtypelen, ndlen;
    char *dtype_str;
    uint32_t dtypelen_off, dtypestr_off, ndlen_off, nddata_off, numFeatures;
    bool hasTheta= true;
    
    // read the dtype
    memcpy(&dtypelen_off, p + sz - sizeof(uint32_t), sizeof(uint32_t));
    ASSERT( dtypelen_off < sz );
    memcpy(&dtypelen, p + dtypelen_off, sizeof(uint32_t));
    dtypestr_off = dtypelen_off + sizeof(uint32_t);
    ASSERT(dtypestr_off + dtypelen <= sz );
    dtype_str = (char*)malloc(dtypelen + 1);
    ASSERT(dtype_str);
    dtype_str[dtypelen] = 0; //'\0'
    // dtype_str== "[('x', '<f4'), ('y', '<f4'), ('a', '<f4'), ('b', '<f4'), ('c', '<f4'), ('theta', '<f4')]";
    if (dtypelen==88){
        hasTheta= true;
    } else if (dtypelen==70){
        hasTheta= false;
    } else {
        std::cout<<dtypelen<<"\n";
        memcpy(dtype_str, p + dtypestr_off, dtypelen*sizeof(char));
        std::cerr<<"Don't know what data is this!? dtype_str="<<dtype_str<<"\n";
        free(dtype_str);
        throw std::runtime_error( "Don't know what data is this!?" );
    }
    
    // read the shape
    ndlen_off = dtypestr_off + dtypelen*sizeof(char);
    nddata_off = ndlen_off + sizeof(uint32_t);
    memcpy(&ndlen, p + ndlen_off, sizeof(uint32_t));
    ASSERT( (nddata_off + ndlen*sizeof(uint32_t))<=sz );
    
    ASSERT(ndlen==1);
    memcpy(&numFeatures, p + nddata_off, sizeof(uint32_t));
    
    
    // read the data
    regions.reserve(numFeatures);
    
    uint32_t stride= 5 + int(hasTheta); // number of fields
    float *x= (float*)&p[0];
    float *y= (float*)&p[  sizeof(float)];
    float *a= (float*)&p[2*sizeof(float)];
    float *b= (float*)&p[3*sizeof(float)];
    float *c= (float*)&p[4*sizeof(float)];
    
    for (uint32_t i=0; i<numFeatures; ++i)
        regions.push_back( ellipse( x[stride*i],y[stride*i],a[stride*i],b[stride*i],c[stride*i] ) );
    
    free(dtype_str);
    
}
