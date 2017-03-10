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

#include <vector>
#include <string>

#include <iostream>
#include <math.h>
#include <stdexcept>

#include <boost/format.hpp>

#include "index_with_data.h"
#include "index_with_data_file.h"
#include "iidx_with_data_builder.h"
#include "util.h"
#include "timing.h"



class fakeComp : public compressorIndep {
    public:
        
        uint32_t
            compress( float const vecs[], uint32_t const n, std::string &data ) const {
                throw std::runtime_error("not implemented");
                return 0;
            }
        
        void
            decompress( std::string const &data, float *&vec ) const {
                throw std::runtime_error("not implemented");
            }
        
        uint32_t numDims() const { throw std::runtime_error("not implemented"); }
        
        virtual charStream*
            charStreamFactoryCreate() const {
                charStream *charStream_obj= charStream::charStreamCreate(8);
                return charStream_obj;
            }
        uint32_t numCodePerVector() const {
            return 1;
        }
        uint32_t
            numBytesPerVector() const { return 1; }
};



int main(){
    
    std::string fidxFn="/home/relja/Relja/Data/Temp/CoarsePQ/fidx_temp.bin";
    std::string iidxFn="/home/relja/Relja/Data/Temp/CoarsePQ/iidx_temp.bin";
    
    // make forward index
    if (true) {
        std::cout<<"creating fidx\n";
        indexWithDataFileBuilder idxBuilder( fidxFn, "Testing index" );
        
        std::vector<uint32_t> vecIDs;
        uint32_t size;
        unsigned char *data;
        
        vecIDs.clear();
        size= 6;
        data= new unsigned char [size];
        for (uint32_t i= 0; i<5; ++i) { vecIDs.push_back(i); data[i]= 20+i; }
        vecIDs.push_back(0); data[5]= 20+0;
        idxBuilder.addData(0, vecIDs, data, size);
        delete []data;
        
        vecIDs.clear();
        size= 4;
        data= new unsigned char [size];
        for (uint32_t i= 0; i<4; ++i) { vecIDs.push_back(10+i); data[i]= 20+i; }
        idxBuilder.addData(1, vecIDs, data, size);
        delete []data;
        
        vecIDs.clear();
        size= 5;
        data= new unsigned char [size];
        for (uint32_t i= 0; i<5; ++i) { vecIDs.push_back(6-i); data[i]= 20+i; }
        idxBuilder.addData(5, vecIDs, data, size);
        delete []data;
        
        idxBuilder.close();
    }
    
    if (true){
        
        std::cout<<"loading fidx\n";
        #if 1
        indexWithDataFile const fidx(fidxFn);
        #else
        indexWithDataFile const fidx_(fidxFn);
        indexWithDataInRam fidx(fidx_);
        #endif
        
        uint32_t N, size;
        unsigned char *data;
        std::vector<uint32_t> vecIDs;
            
        for (uint32_t ID= 0; ID < fidx.numIDs(); ++ID){
            
            N= fidx.getNumWithID(ID);
            
            std::cout<<"\n"<<ID<<" "<< N <<": ";
            fidx.getData(ID, vecIDs, data, size);
            
            for (uint32_t i= 0; i<vecIDs.size(); ++i)
                std::cout<< vecIDs[i]<<" ";
            std::cout<<"\n";
            
            for (uint32_t i= 0; i<size; ++i)
                std::cout<< static_cast<uint32_t>(data[i])<<" ";
            std::cout<<"\n";
            
            delete []data;
        }
        
    }
    
    if (true){
        std::cout<<"creating iidx\n";
        
        indexWithDataFile const fidx(fidxFn);
        fakeComp fakeComp_obj;
        iidxWithDataBuilder::build(fidx, iidxFn, fakeComp_obj, "/home/relja/Relja/Data/Temp/CoarsePQ");
    }
    
    if (true){
        
        std::cout<<"loading iidx\n";
        #if 1
        indexWithDataFile const iidx(iidxFn);
        #else
        indexWithDataFile const iidx_(iidxFn);
        indexWithDataInRam iidx(iidx_);
        #endif
        
        uint32_t N, size;
        unsigned char *data;
        std::vector<uint32_t> docIDs;
            
        for (uint32_t vecID= 0; vecID < iidx.numIDs(); ++vecID){
            
            N= iidx.getNumWithID(vecID);
            
            std::cout<<"\n"<<vecID<<" "<< N <<": ";
            iidx.getData(vecID, docIDs, data, size);
            
            for (uint32_t i= 0; i<docIDs.size(); ++i)
                std::cout<< docIDs[i]<<" ";
            std::cout<<"\n";
            
            for (uint32_t i= 0; i<size; ++i)
                std::cout<< static_cast<uint32_t>(data[i])<<" ";
            std::cout<<"\n";
            
            delete []data;
        }
        
    }
    
    return 0;
    
}
