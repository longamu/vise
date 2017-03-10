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

#ifndef _RELJA_MODEL_FITTER_H_
#define _RELJA_MODEL_FITTER_H_

#include <vcl_vector.h>
#include <vcl_utility.h>
#include <vnl/vnl_random.h>

#include <stdint.h>

//using namespace std;


class model_fitter {
    
    public:

        virtual uint32_t
            getNumberToMinimalFit()=0;

        virtual uint32_t
            getNumberToFinalFit()=0;
            
        virtual uint32_t
            fitMinimalModel( vcl_vector< vcl_pair<uint32_t, uint32_t> > &samples )=0;
            
        virtual uint32_t
            fitFinalModel( uint32_t nReest )=0;
            
        uint32_t nFindInliers;
        
        uint32_t bestNInliers;
        vcl_vector< vcl_pair<uint32_t, uint32_t> > bestInliers;
        
        virtual ~model_fitter(){};
        
};

#endif //_RELJA_MODEL_FITTER_H_
