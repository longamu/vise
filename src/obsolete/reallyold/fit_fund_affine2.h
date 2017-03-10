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

#ifndef _RELJA_FUND_AFFINE2_H_
#define _RELJA_FUND_AFFINE2_H_

#include "fit_fund_affine4.h"
#include "ellipse.h"
#include "homography.h"

#include "solve_quartic_methods.h"

#include <vcl_cstdio.h>
#include <vcl_iostream.h>
#include <vcl_vector.h>
#include <vcl_utility.h>
#include <vcl_cmath.h>
#include <vnl/vnl_math.h>

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_vector.h>
#include <vnl/algo/vnl_svd_economy.h>
#include <vgl/vgl_homg_point_2d.h>

#include <stdint.h>

class fit_fund_affine2 : public fit_fund_affine4 {
    
    protected:
        
        int debug_;
        
        // find and apply an affine transformation (Haff) to ellipse1 and ellipse2 such that ellipse1transf is a unit circle with centre at (0,0), and the centre of ellipse2transf has x=0
        static void
            affineNormalize(
                ellipse &ellipse1, ellipse &ellipse2,
                ellipse &ellipse2transf,
                vgl_h_matrix_2d<double> &Haff );
    
    public:
        
        fit_fund_affine2(
                   vcl_vector< ellipse > aEllipses1, vcl_vector< ellipse > aEllipses2,
                   vcl_vector< vcl_pair<uint32_t, uint32_t> > &aPutativeMatches,
                   double aErrorThres, double abRatioLimit,
                   bool aVerbose= false ):
                       fit_fund_affine4(
                           aEllipses1, aEllipses2,
                           aPutativeMatches,
                           aErrorThres, abRatioLimit,
                           aVerbose )
                       { debug_=0; }
        
        virtual uint32_t
            fitMinimalModel( vcl_vector< vcl_pair<uint32_t, uint32_t> > &samples );
        
        virtual inline uint32_t
            getNumberToMinimalFit()
                { return 2; }
        
};


#endif //_RELJA_FUND_AFFINE2_H_
