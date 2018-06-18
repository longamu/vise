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

#ifndef _RELJA_FIT_AFFINE_H_
#define _RELJA_FIT_AFFINE_H_

#include "model_fitter.h"
#include "homography.h"
#include "ellipse.h"

#include <vcl_cstdio.h>
#include <vcl_iostream.h>
#include <vcl_vector.h>
#include <vcl_utility.h>
#include <vcl_cmath.h>

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_det.h>
#include <vnl/vnl_vector.h>
#include <vnl/algo/vnl_svd_economy.h>
#include <vgl/vgl_homg_point_2d.h>
#include <vgl/vgl_distance.h>
#include <vgl/algo/vgl_norm_trans_2d.h>

#include <stdint.h>

#include <math.h> //temp for isnan


class fit_affine: public model_fitter {
    
    protected:
        
        static void
            getHnorm(
                vcl_vector< vgl_homg_point_2d<double> > &inlierPoints1,
                vcl_vector< vgl_homg_point_2d<double> > &inlierPoints2,
                vgl_norm_trans_2d<double> &normTrans1,
                vgl_norm_trans_2d<double> &normTrans2,
                vnl_matrix_fixed<double,3,3> &Hnorm );
                
        bool verbose;
        
        vcl_vector< ellipse > ellipses1, ellipses2;
        vcl_vector< vcl_pair<uint32_t, uint32_t> > putativeMatches;
        
        double errorThresSq, lowAreaChangeSq, highAreaChangeSq;
        
        vcl_vector< uint32_t > point1Used, point2Used;
        
    public:
        
        fit_affine( vcl_vector< ellipse > aEllipses1, vcl_vector< ellipse > aEllipses2,
                    vcl_vector< vcl_pair<uint32_t, uint32_t> > &aPutativeMatches,
                    double aErrorThres, double aLowAreaChange, double aHighAreaChange,
                    bool aVerbose= false );
                    
        ~fit_affine();
        
        inline uint32_t
            getNumberToMinimalFit()
                { return 1; }

        inline uint32_t
            getNumberToFinalFit()
                { return 3; }

        uint32_t
            fitMinimalModel( vcl_vector< vcl_pair<uint32_t, uint32_t> > &samples );
            
        uint32_t
            fitFinalModel( uint32_t nReest );
        
        uint32_t
            findInliers( homography &h, vcl_vector< vcl_pair<uint32_t, uint32_t> > &inliers, bool returnInliers );
        
        static void
            getH(
                vcl_vector< vgl_homg_point_2d<double> > &inlierPoints1,
                vcl_vector< vgl_homg_point_2d<double> > &inlierPoints2,
                vgl_h_matrix_2d<double> &H );
        
        homography *bestH;
        
};


#endif //_RELJA_FIT_AFFINE_H_