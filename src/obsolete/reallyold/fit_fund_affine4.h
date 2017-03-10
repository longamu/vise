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

#ifndef _RELJA_FUND_AFFINE4_H_
#define _RELJA_FUND_AFFINE4_H_

#include "model_fitter.h"
#include "ellipse.h"

#include <vcl_cstdio.h>
#include <vcl_iostream.h>
#include <vcl_vector.h>
#include <vcl_utility.h>
#include <vcl_cmath.h>

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_vector.h>
#include <vnl/algo/vnl_svd_economy.h>
#include <vgl/vgl_homg_point_2d.h>

#include <stdint.h>

class fit_fund_affine4 : public model_fitter {
    
    protected:
        
        bool verbose;
        
        vcl_vector< ellipse > ellipses1, ellipses2;
        vcl_vector< vcl_pair<uint32_t, uint32_t> > putativeMatches;
        
        double errorThres, abRatioLimitSq;
        
        vcl_vector< uint32_t > point1Used, point2Used;
            
    public:
        
        fit_fund_affine4(
                   vcl_vector< ellipse > aEllipses1, vcl_vector< ellipse > aEllipses2,
                   vcl_vector< vcl_pair<uint32_t, uint32_t> > &aPutativeMatches,
                   double aErrorThres, double abRatioLimit,
                   bool aVerbose= false );
                   
        ~fit_fund_affine4();
                
        virtual inline uint32_t
            getNumberToMinimalFit()
                { return 4; }

        virtual inline uint32_t
            getNumberToFinalFit()
                { return 4; }
            
        uint32_t
            fit( vcl_vector< vcl_pair<uint32_t, uint32_t> > &matches, bool aFinalFit );
            
        virtual uint32_t
            fitMinimalModel( vcl_vector< vcl_pair<uint32_t, uint32_t> > &samples );
            
        virtual uint32_t
            fitFinalModel( uint32_t nReest );
        
        virtual uint32_t
            findInliers( vnl_matrix_fixed<double,3,3> &F,
                         vcl_vector< vcl_pair<uint32_t, uint32_t> > &inliers, bool returnInliers );
        
        vnl_matrix_fixed<double,3,3> bestF;
};


#endif //_RELJA_FUND_AFFINE4_H_
