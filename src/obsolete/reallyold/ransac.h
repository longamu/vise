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

#ifndef _RELJA_RANSAC_H_
#define _RELJA_RANSAC_H_

#include <vcl_cstdio.h>
#include <vcl_cmath.h>
#include <vcl_vector.h>
#include <vcl_utility.h>
#include <vnl/vnl_random.h>

#include <stdint.h>

#include "model_fitter.h"

using namespace std;


class ransac {
    
    private:
        
        inline static uint32_t
            getNStopping( double pOutlier, double pFail, unsigned int numberToFit );
        
        template <class T>
        inline static void
            swap( vcl_vector<T> &array, uint32_t i, uint32_t j );
                
    public:
        
        static bool
            doRansac(
                vcl_vector< vcl_pair<uint32_t, uint32_t> > &putativeMatches,
                model_fitter &modelFitter,
                uint32_t nReest= 4,
                uint32_t randomSeed= 0,
                bool verbose= false );
        
        static vcl_vector< vcl_pair<uint32_t, uint32_t> >*
            uniformSample(
                uint32_t nSamples, vcl_vector< vcl_pair<uint32_t, uint32_t> > &putativeMatches, 
                vnl_random &randomGen );
            
};

#endif //_RELJA_RANSAC_H_