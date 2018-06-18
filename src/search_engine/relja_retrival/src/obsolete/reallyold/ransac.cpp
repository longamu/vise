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

#include "ransac.h"
#include "model_fitter.h"

#include <vcl_cstdio.h>
#include <vcl_cmath.h>
#include <vcl_algorithm.h>
#include <vcl_vector.h>
#include <vcl_utility.h>
#include <vnl/vnl_random.h>

#include <stdint.h>


uint32_t
ransac::getNStopping( double pOutlier, double pFail, uint32_t numberToFit ){
    if ( pOutlier > 0 )
        return vcl_min(
            vcl_ceil( vcl_log( pFail ) / vcl_log( 1- vcl_pow( (1-pOutlier), static_cast<double>(numberToFit)) ) ),
            100000.0
        );
    return 0;
}


template <class T>
void
ransac::swap( vcl_vector<T> &array, uint32_t i, uint32_t j ){
    T temp= array[j];
    array[j]= array[i];
    array[i]= temp;
}


vcl_vector< vcl_pair<uint32_t, uint32_t> >*
ransac::uniformSample( 
    uint32_t nSamples, vcl_vector< vcl_pair<uint32_t, uint32_t> > &putativeMatches, 
    vnl_random &randomGen ){
    
    uint32_t nPutativeMatches= putativeMatches.size();
    uint32_t newSample;
    
    vcl_vector< vcl_pair<uint32_t, uint32_t> > *samples= new vcl_vector< vcl_pair<uint32_t, uint32_t> > ();
    samples->reserve(nSamples);
    
    for( uint32_t iSample=0; iSample < nSamples; ++iSample ){
        newSample = randomGen.lrand32( 0, nPutativeMatches-iSample-1 );
        samples->push_back( vcl_pair<uint32_t, uint32_t>( putativeMatches[newSample] ) );
        swap( putativeMatches, newSample, nPutativeMatches-iSample-1 );
    }
    
    return samples;
    
}


bool
ransac::doRansac(
    vcl_vector< vcl_pair<uint32_t, uint32_t> > &putativeMatches,
    model_fitter &modelFitter,
    uint32_t nReest,
    uint32_t randomSeed,
    bool verbose ){
 
    uint32_t nPutativeMatches= putativeMatches.size();
    uint32_t numberToMinimalFit= modelFitter.getNumberToMinimalFit();
    uint32_t numberToFinalFit= modelFitter.getNumberToFinalFit();
    
    // check if we have enough putative matches
    if ( nPutativeMatches < numberToFinalFit )
        return false;
    
    // initialize random generator
    vnl_random randomGen;
    if (randomSeed<0)
        randomGen= vnl_random();
    else
        randomGen= vnl_random( randomSeed );
    
    // other initializations
    double pOutlier= 1.0- (0.0+numberToFinalFit)/nPutativeMatches; //0.99;
    double pFail= 0.01;
    
    uint32_t bestNInliers=0, nInliers=0;
    
    uint32_t nIter=0, nStopping= ransac::getNStopping(pOutlier, pFail, numberToMinimalFit);//1;
    
    vcl_vector< vcl_pair<uint32_t, uint32_t> >* samples= NULL;
    
    // start RANSAC
    
    while ( nIter < nStopping ){
        
        
        samples= ransac::uniformSample( numberToMinimalFit, putativeMatches, randomGen );
        nInliers= modelFitter.fitMinimalModel( *samples );
        delete samples;
        samples= NULL;
        
        ++nIter;
        if (nInliers >= numberToFinalFit){
            // model successfully fitted
            
            if (nInliers > bestNInliers){
                bestNInliers= nInliers;
                pOutlier= min( 1 - (0.0 + bestNInliers)/nPutativeMatches, pOutlier );
                nStopping= ransac::getNStopping( pOutlier, pFail, numberToMinimalFit );
            }
            
        }
        
        //vcl_printf("\t\tRANSAC inliers %d %d\n", nInliers, bestNInliers);
        
    }
    
    if (verbose)
        vcl_printf("RANSAC: Number of iterations= %d\n", nIter);
    
    bestNInliers= modelFitter.fitFinalModel( nReest );
    
    bool success= (bestNInliers >= numberToFinalFit);
    if (verbose){
        if (success)
            vcl_printf("RANSAC: Successfully found a model\n");
        else
            vcl_printf("RANSAC: Failed to find a model\n");
    }
    
    return success; 
    
}
