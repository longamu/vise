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

#include <boost/format.hpp>

#include "product_quant.h"
#include "desc_wrapper_jp_db5.h"
#include "desc_from_file_to_hell.h"
#include "desc_from_fvecs_file.h"
#include "nn_evaluator.h"
#include "nn_compressed.h"
#include "coarse_residual.h"
#include "index_with_data.h"
#include "index_with_data_file.h"
#include "index_with_data_file_fixed1.h"
#include "iidx_with_data_builder.h"
#include "util.h"
#include "timing.h"



int sift1M(bool doCompress= false, bool doMakeIndex= false, bool doTest= true, uint32_t nCoarseK= 1024, uint32_t nSubQuant= 8, uint32_t subQuantK= 256, uint32_t w= 8){
    
    std::string coarseClstFn= (boost::format("/home/relja/Relja/Data/Temp/CoarsePQ/clst_sift_learn_%d_43.bin") % nCoarseK ).str();
    std::vector<std::string> clstFns(nSubQuant);
    for (unsigned i= 0; i<nSubQuant; ++i)
        clstFns[i]= (boost::format("/home/relja/Relja/Data/Temp/CoarsePQ/clst_sift_learn_%d_%d_43_residual_%d_43_pq%02d.bin") % nSubQuant % subQuantK % nCoarseK % i ).str();
    
    productQuant pq(clstFns, true);
    
    descFromFvecsFile descFile("/home/relja/Relja/Data/Jegou_ANN/ANN_SIFT1M/sift_base.fvecs");
    
    std::string fidxFn= (boost::format("/home/relja/Relja/Data/Temp/CoarsePQ/sift1M_fidx_residual_%d_43_pq_%d_%d_43.bin") % nCoarseK % nSubQuant % subQuantK ).str();
    
    std::string iidxFn= (boost::format("/home/relja/Relja/Data/Temp/CoarsePQ/sift1M_iidx_residual_%d_43_pq_%d_%d_43.bin") % nCoarseK % nSubQuant % subQuantK ).str();
    
    if (doCompress){
        // compress the whole set
        #if 0
        coarseResidualFidxBuilder::buildFromFile( fidxFn, coarseClstFn, pq, descFile, false );
        #else
        indexWithDataFileFixed1Builder fidxBuilder(fidxFn, pq.numBytesPerVector() );
        coarseResidualFidxBuilder::buildFromFile( fidxBuilder, coarseClstFn, pq, descFile, false );
        #endif
    }
    
    
    if (doMakeIndex){
        #if 0
        indexWithDataFile const fidx(fidxFn);
        #else
        indexWithDataFileFixed1 const fidx(fidxFn);
        #endif
        
        // make iidx
        iidxWithDataBuilder::build(fidx, iidxFn, pq, "/home/relja/Relja/Data/Temp/CoarsePQ");
    }
    
    
    if (doTest){
        
        // find KNN
        
        nnEvaluator nnEvaluator_obj;
        
        #if 1
        indexWithDataFile const iidx(iidxFn);
        #else
        indexWithDataFile const iidx_(iidxFn);
        indexWithDataInRam iidx(iidx_);
        #endif
        
        coarseResidual coarseResidual_obj( coarseClstFn, pq, iidx, w, false );
        double recAt100= nnEvaluator_obj.computeAverageRec( coarseResidual_obj, 100, NULL, false, true );
        std::cout<<"rec@100= "<< recAt100<<"\n";
        
    }
    
    
    return 0;
}



int testIdx(){
    
    std::string idxFn="/home/relja/Relja/Data/Temp/CoarsePQ/idx_temp.bin";
    
    if (true) {
        indexWithDataFileBuilder idxBuilder( idxFn, "Testing index" );
        
        std::vector<uint32_t> vecIDs;
        uint32_t size;
        unsigned char *data;
        
        vecIDs.clear();
        for (uint32_t i= 0; i<5; ++i) vecIDs.push_back(i);
        size= 3;
        data= new unsigned char [size];
        data[0]=0; data[1]=1; data[2]=2;
        idxBuilder.addData(0, vecIDs, data, size);
        delete []data;
        
        vecIDs.clear();
        for (uint32_t i= 0; i<4; ++i) vecIDs.push_back(10+i);
        size= 4;
        data= new unsigned char [size];
        data[0]=0; data[1]=1; data[2]=2; data[3]= 3;
        idxBuilder.addData(1, vecIDs, data, size);
        delete []data;
        
        #if 0
        vecIDs.clear();
        for (uint32_t i= 0; i<5; ++i) vecIDs.push_back(i);
        size= 3;
        data= new unsigned char [size];
        data[0]=0; data[1]=1; data[2]=2;
        idxBuilder.addData(0, vecIDs, data, size);
        delete []data;
        #endif
        
        vecIDs.clear();
        for (uint32_t i= 0; i<5; ++i) vecIDs.push_back(i);
        size= 3;
        data= new unsigned char [size];
        data[0]=0; data[1]=1; data[2]=2;
        idxBuilder.addData(5, vecIDs, data, size);
        delete []data;
        
        idxBuilder.close();
    }
    
    if (true){
        
        #if 0
        indexWithDataFile const idx(idxFn);
        #else
        indexWithDataFile const idx_(idxFn);
        indexWithDataInRam idx(idx_);
        #endif
        
        uint32_t N, size;
        unsigned char *data;
        std::vector<uint32_t> vecIDs;
            
        for (uint32_t ID= 0; ID < idx.numIDs(); ++ID){
            
            N= idx.getNumWithID(ID);
            
            std::cout<<"\n"<<ID<<" "<< N <<"\n";
            idx.getData(ID, vecIDs, data, size);
            
            for (uint32_t i= 0; i<vecIDs.size(); ++i)
                std::cout<< vecIDs[i]<<" ";
            std::cout<<"\n";
            
            for (uint32_t i= 0; i<size; ++i)
                std::cout<< static_cast<uint32_t>(data[i])<<" ";
            std::cout<<"\n";
            
            delete []data;
        }
        
    }
    
    return 0;
    
}



int testIdx1(){
    
    std::string idxFn="/home/relja/Relja/Data/Temp/CoarsePQ/idx_temp1.bin";
    
    if (true) {
//         indexWithDataFileBuilder idxBuilder( idxFn, "Testing index" );
        indexWithDataFileFixed1Builder idxBuilder( idxFn, 3, "Testing index" );
        
        std::vector<uint32_t> vecIDs;
        uint32_t size;
        unsigned char *data;
        
        vecIDs.clear();
        for (uint32_t i= 0; i<1; ++i) vecIDs.push_back(i);
        size= 3;
        data= new unsigned char [size];
        data[0]=0; data[1]=1; data[2]=2;
        idxBuilder.addData(0, vecIDs, data, size);
        delete []data;
        
        vecIDs.clear();
        for (uint32_t i= 0; i<1; ++i) vecIDs.push_back(10+i);
        size= 3;
        data= new unsigned char [size];
        data[0]=0; data[1]=1; data[2]=2;;
        idxBuilder.addData(1, vecIDs, data, size);
        delete []data;
        
        #if 0
        vecIDs.clear();
        for (uint32_t i= 0; i<5; ++i) vecIDs.push_back(i);
        size= 3;
        data= new unsigned char [size];
        data[0]=0; data[1]=1; data[2]=2;
        idxBuilder.addData(0, vecIDs, data, size);
        delete []data;
        #endif
        
        vecIDs.clear();
        for (uint32_t i= 0; i<1; ++i) vecIDs.push_back(i);
        size= 3;
        data= new unsigned char [size];
        data[0]=0; data[1]=1; data[2]=2;
        idxBuilder.addData(5, vecIDs, data, size);
        delete []data;
        
        idxBuilder.close();
    }
    
    if (true){
        
        #if 0
        indexWithDataFileFixed1 const idx(idxFn);
        #else
        indexWithDataFile const idx_(idxFn);
        indexWithDataInRam idx(idx_);
        #endif
        
        uint32_t N, size;
        unsigned char *data;
        std::vector<uint32_t> vecIDs;
            
        for (uint32_t ID= 0; ID < idx.numIDs(); ++ID){
            
            N= idx.getNumWithID(ID);
            
            std::cout<<"\n"<<ID<<" "<< N <<"\n";
            idx.getData(ID, vecIDs, data, size);
            
            for (uint32_t i= 0; i<vecIDs.size(); ++i)
                std::cout<< vecIDs[i]<<" ";
            std::cout<<"\n";
            
            for (uint32_t i= 0; i<size; ++i)
                std::cout<< static_cast<uint32_t>(data[i])<<" ";
            std::cout<<"\n";
            
            delete []data;
        }
        
    }
    
    return 0;
    
}



int main(int argc, char *argv[]){
//     return testIdx();
//     return testIdx1();
    return sift1M( argc>1 && argv[1][0]=='c' , argc>1 && argv[1][1]=='i', argc>1 && argv[1][2]=='t', argc>2 ? atoi(argv[2]) : 1024, argc>3 ? atoi(argv[3]) : 8, argc>4 ? atoi(argv[4]) : 256, argc>5 ? atoi(argv[5]) : 8 );
}
