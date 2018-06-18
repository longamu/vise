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

#ifndef _FIT_AFFINE3_H_
#define _FIT_AFFINE3_H_

#include <vector>
#include <stdint.h>

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_det.h>
#include <vnl/vnl_vector.h>
#include <vnl/algo/vnl_svd_economy.h>
#include <vgl/vgl_homg_point_2d.h>
#include <vgl/vgl_distance.h>
#include <vgl/algo/vgl_norm_trans_2d.h>

#include <stdint.h>
#include <math.h>

#include "model_fitter.h"
#include "homography.h"


class fitAffine3 : public model_fitter {
    
    public:
        
        fitAffine3( std::vector< vgl_homg_point_2d<double> > &aPoints1, std::vector< vgl_homg_point_2d<double> > &aPoints2, std::vector< std::pair<uint32_t, uint32_t> > &aPutativeMatches, double aErrorThres);
        
        virtual
            ~fitAffine3();
        
        uint32_t
            getNumberToMinimalFit(){ return 3; }
        
        uint32_t
            getNumberToFinalFit(){ return 3; }
        
        uint32_t
            fitMinimalModel( std::vector< std::pair<uint32_t, uint32_t> > &samples );
        
        uint32_t
            fitFinalModel( uint32_t nReest );
        
        uint32_t
            findInliers( homography &h, bool returnInliers= false, std::vector< std::pair<uint32_t, uint32_t> > *inliers= NULL );
        
        homography *bestH;
        
    private:
        
        std::vector< std::pair<uint32_t, uint32_t> > *putativeMatches;
        std::vector< vgl_homg_point_2d<double> > *points1, *points2;
        
        std::vector< uint32_t > point1Used, point2Used;
        
        double errorThresSq;
    
};

#endif
