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
#include "nn_evaluator.h"
#include "coarse_residual.h"
#include "index_with_data_file.h"
#include "index_with_data_file_fixed1.h"
#include "nn_single_retriever.h"
#include "util.h"
#include "timing.h"



int main(){
    
    uint32_t nCoarseK= 1024;
    uint32_t nSubQuant= 8;
    uint32_t subQuantK= 256;
    uint32_t w= 8;
    
    std::string coarseClstFn= (boost::format("/home/relja/Relja/Data/Temp/CoarsePQ/clst_sift_learn_%d_43.e3bin") % nCoarseK ).str();
    std::vector<std::string> clstFns(nSubQuant);
    for (unsigned i= 0; i<nSubQuant; ++i)
        clstFns[i]= (boost::format("/home/relja/Relja/Data/Temp/CoarsePQ/clst_sift_learn_%d_%d_43_residual_%d_43_pq%02d.e3bin") % nSubQuant % subQuantK % nCoarseK % i ).str();
    
    productQuant pq(clstFns, true);
    
    std::string fidxFn= (boost::format("/home/relja/Relja/Data/Temp/CoarsePQ/sift1M_fidx_residual_%d_43_pq_%d_%d_43.bin") % nCoarseK % nSubQuant % subQuantK ).str();
    std::string iidxFn= (boost::format("/home/relja/Relja/Data/Temp/CoarsePQ/sift1M_iidx_residual_%d_43_pq_%d_%d_43.bin") % nCoarseK % nSubQuant % subQuantK ).str();
    
    indexWithDataFileFixed1 const fidx(fidxFn);
    
    #if 0
    indexWithDataFile const iidx(iidxFn);
    #else
    indexWithDataFile const iidx_(iidxFn);
    indexWithDataInRam iidx(iidx_);
    #endif
    
    coarseResidual coarseResidual_obj( coarseClstFn, pq, iidx, w, false );
    
    nnSingleRetriever nnSR( &fidx, coarseResidual_obj );
    
    std::vector<indScorePair> queryRes;
    nnSR.internalQuery(0, queryRes);
    
    return 0;
}
