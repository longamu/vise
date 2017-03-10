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

// #include "document_map.h"
// #include "desc_wrapper_jp_db5.h"
// #include "desc_from_file_to_hell.h"
#include "desc_from_fvecs_file.h"
#include "nn_evaluator.h"
#include "nn_compressed.h"
#include "char_streams.h"
#include "util.h"
#include "timing.h"
#include "macros.h"



int f2(bool doCompress= false, bool doTest= true, uint32_t nSubQuant= 8, uint32_t subQuantK= 256){
    
    std::vector<std::string> clstFns(nSubQuant);
    
    #if 1
    std::string clstFnsTemplate= util::expandUser("~/Relja/Data/Temp/PQ/clst_sift_learn_%d_%d_43_pq%02d.e3bin");
    std::string baseFn= util::expandUser("~/Relja/Data/Jegou_ANN/ANN_SIFT1M/sift_base.fvecs");
    std::string queryFn= util::expandUser("~/Relja/Data/Jegou_ANN/ANN_SIFT1M/sift_query.fvecs");
    std::string pqFn= util::expandUser( (boost::format("~/Relja/Data/Temp/PQ/sift1M_pq_%d_%d_43.bin") % nSubQuant % subQuantK).str() );
    #else
    std::string clstFnsTemplate= util::expandUser( "~/Relja/Data/Temp/OPQ/clst_sift_learn_%d_%d_43_opq%02d.e3bin" );
    std::string baseFn= util::expandUser( (boost::format("~/Relja/Data/Temp/OPQ/sift_base_subQuant%d.fvecs") % nSubQuant).str() );
    std::string queryFn= util::expandUser( (boost::format("~/Relja/Data/Temp/OPQ/sift_query_subQuant%d.fvecs") % nSubQuant).str() );
    std::string pqFn= util::expandUser( (boost::format("~/Relja/Data/Temp/OPQ/sift1M_opq_%d_%d_43.bin") % nSubQuant % subQuantK).str() );
    #endif
    
    for (unsigned i= 0; i<nSubQuant; ++i)
        clstFns[i]= (boost::format(clstFnsTemplate) % nSubQuant % subQuantK % i ).str();
    
    productQuant pq(clstFns, true);
    
    descFromFvecsFile descFile(baseFn.c_str());
    uint32_t numDescs= descFile.numDocs();
    
    if (doCompress){
        
        // compress the whole set
        
        uint32_t numDescs_;
        
//         numDescs= 1000;
        
        std::cout<<"reading\n";
        
        float *allDescs= new float[numDescs*128];
        float *descIter= allDescs;
        
        for (uint32_t docID= 0; docID<numDescs; ++docID){
            if (docID%100000==0)
                std::cout<<"docID= "<<docID<<" / "<<numDescs<<"\n";
            float *thisDesc;
            descFile.getDescs(docID, numDescs_, thisDesc);
            ASSERT( numDescs_==1 );
            std::memcpy(descIter, thisDesc, 128*sizeof(float));
            delete []thisDesc;
            descIter+= 128;
        }
        
        std::cout<<"quantizing\n";
        std::string data;
        double t0= timing::tic();
        
        uint32_t size= pq.compress(allDescs, numDescs, data);
        std::cout<< "done, "<< round(timing::toc(t0)/1000)<<" s\n";
        delete []allDescs;
        
        // save to file
        FILE *f= fopen(pqFn.c_str(), "wb");
        fwrite(data.c_str(), 1, size, f);
        fclose(f);
        
    }
    
    
    
    if (doTest){
        
        // find KNN
        
        uint32_t size= util::fileSize(pqFn);
        
        // load compressed from file
        FILE *f= fopen(pqFn.c_str(), "rb");
        
        std::string data;
        data.resize(size);
        uint32_t temp_= fread(&data[0], 1, size, f);
        if (false && temp_) {} // to avoid the warning about not checking temp_
        
        fclose(f);
        nnEvaluator nnEvaluator_obj(queryFn);
        nnCompressed nnCompressed_obj( pq, data, size );
        double recAt100= nnEvaluator_obj.computeAverageRec( nnCompressed_obj, 100, NULL, false, true );
        std::cout<<"rec@100= "<< recAt100<<"\n";
        
    }
    
    
    return 0;
}



/*
int f1(){
    
    std::vector<std::string> clstFns(8);
    for (int i= 0; i<8; ++i)
        clstFns[i]= (boost::format("/home/relja/Relja/Data/Temp/OxfordTiny/clst_oxTiny_hesaff_rootsift_256_43_pq%02d.bin") % i ).str();
    
    productQuant pq(clstFns, true);
    
    if (true) {
    
    float vecs[128+128];
    for (int i=0; i<128; ++i){
        vecs[i]= 0;
        vecs[i+128]= 0.2;
    }
    
    std::string data;
    float *vecs_de;
    float *ds1, *ds2;
    
    pq.compress(vecs, 2, data);
    pq.decompress(data, vecs_de);
    pq.getDistsSq(vecs, data, ds1);
    pq.getDistsSq(vecs+128, data, ds2);
    
    //std::cout<<"decomp:\n";
    //for (int i=0; i<2; ++i){
    //    for (int d=0; d<128; ++d)
    //        std::cout<<vecs_de[i*128+d]<<" ";
    //    std::cout<<"\n";
    //}
    
    for (int i=0; i<2; ++i)
        std::cout<<ds1[i]<<" ";
    std::cout<<"\n";
    for (int i=0; i<2; ++i)
        std::cout<<ds2[i]<<" ";
    std::cout<<"\n";
    
    delete []ds1;
    delete []ds2;
    delete []vecs_de;
    
    }
    
    if (false) {
    
    uint32_t numDims= 128;
    
    documentMap docMap( "/home/relja/Relja/Data/Temp/OxfordTiny/dset_oxTiny.db", "oxTiny" );
    
    descWrapperJpDb5<uint8_t> desc_jpdb5("/home/relja/Relja/Data/Temp/OxfordTiny/feat_oxTiny_hesaff_sift.h5", numDims, &docMap);
    descFromFileToHell descFile(desc_jpdb5);
    
    uint32_t numDescs;
    float **descs, *descsFlat;
    
    descFile.getDescs(0, numDescs, descs);
    util::matrixToFlat(numDescs, numDims, descs, descsFlat);
    util::del( numDescs, descs );
    
//     std::cout<<numDescs<<"\n";
    
    std::string data;
    pq.compress(descsFlat, numDescs, data);
    
    float *descsDecomp;
    
    pq.decompress(data, descsDecomp);
    
    float *dsSq;
    
    for (uint32_t i= 0; i<numDescs; ++i){
        
        pq.getDistsSq(descsFlat+i*numDims, data, dsSq);
        
//         for (uint32_t j= 0; j<numDescs; ++j)
//             std::cout<<dsSq[j]<<" ";
//         std::cout<<"\n";
        
        delete []dsSq;
    }
    
    delete []descsDecomp;
    delete []descsFlat;
    
    }
    
    return 0;
}
*/



int main(int argc, char *argv[]){
//     return f1();
    return f2( argc>1 && argv[1][0]=='c' , argc>1 && argv[1][1]=='t', argc>2 ? atoi(argv[2]) : 8, argc>3 ? atoi(argv[3]) : 256 );
}
