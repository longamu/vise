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

#include "soft_assigner_llc.h"

#include <cv.h>
#include <math.h>


void
SA_LLC::getWeights( quantDesc &ww, float *feat ){
    
    uint iNN, KNN= ww.rep.size(), nDims= clstC->numDims, iDim;
    
    cv::Mat wordsX( KNN, nDims, cv::DataType<double>::type );
    
    uint32_t wordID;
    
    // do LLC
    for (iNN= 0; iNN<KNN; ++iNN){
        wordID= ww.rep[iNN].first;
        for (iDim= 0; iDim<nDims; ++iDim)
            wordsX.at<double>(iNN,iDim)= static_cast<double>( clstC->clstC[wordID][iDim] );
    }
    
    cv::Mat dist( KNN, nDims, cv::DataType<double>::type );
    for (uint iNN=0; iNN<KNN; ++iNN)
        for (uint iDim=0; iDim<nDims; ++iDim){
            dist.at<double>(iNN,iDim)= wordsX.at<double>(iNN,iDim) - feat[iDim];
        }
    
    cv::Mat distSq= dist * dist.t();
    
    #if 0
    double traceD= cv::trace(distSq)[0]; // different from what they wrote in the paper - this is from their code
    #endif
    #if 0
    double traceD= cv::trace(d)[0] / sigmaSq; // or normalize with sigma^2
    #endif
    
    #if 1
    // or actually do exp( d^2/sigma^2 ) which is what they say in the paper
    double traceD= 0.0;
    for (uint iNN=0; iNN<KNN; ++iNN)
        traceD+= exp( distSq.at<double>(iNN,iNN) / sigmaSq );
    #endif
    
    cv::Mat II= cv::Mat::eye(KNN, KNN, cv::DataType<double>::type);
    
    cv::Mat C= distSq + lambda* traceD * II ;
    cv::Mat w= C.inv() * cv::Mat::ones( KNN, 1, cv::DataType<double>::type );
    w= w/cv::sum(w)[0];
    
    // remove small components and renormalize
    for (uint i=0; i<KNN; ++i)
        if (w.at<double>(i,0)<0.01)
            w.at<double>(i,0)= 0.0;
    w= w/cv::sum(w)[0];
    
    // output
    for (iNN= 0; iNN<KNN; ++iNN)
        ww.rep[iNN].second= static_cast<float>( w.at<double>(iNN,0) );
    
}
