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

#ifndef _RANSAC_RELJA_RETRIEVAL_H_
#define _RANSAC_RELJA_RETRIEVAL_H_

#include "ransac.h"
#include "fit_affine.h"
#include "fit_fund_affine4.h"
#include "ellipse.h"
#include "putative.h"

#include "jp_dist2.hpp"

#include <vcl_vector.h>
#include <vcl_utility.h>
#include <vcl_cstdio.h>
#include <vcl_string.h>
#include <vcl_cmath.h>

#include <vector>
#include <limits>
#include <stdint.h>



uint32_t
ransac_quant(
        vcl_vector<uint32_t> &ids1,
        vcl_vector< ellipse > &ellipses1,
        vcl_vector<uint32_t> &ids2,
        vcl_vector< ellipse > &ellipses2,
        
        int max_cor,
        float error_thresh,
        float low_area_change,
        float high_area_change,
        int max_reest,
        
        float* best_h,
        vector< pair<uint32_t,uint32_t> >& best_cor
        );



template<class DescType>
uint32_t
ransac_desc(
        const DescType* desc1,
        vcl_vector< ellipse > &ellipses1,
        const DescType* desc2,
        vcl_vector< ellipse > &ellipses2,
        uint32_t ndims,
        
        float error_thresh,
        float low_area_change,
        float high_area_change,
        int max_reest,
        float epsilon, // Used for distance threshold test.
        float deltaSq, // Used for Lowe's second NN test.
        int use_lowe, // Select Lowe method (=1) or distance threshold (=0)
        
        float* best_h,
        std::vector< std::pair<uint32_t, uint32_t> >& best_cor,
        float& weighted_inliers,
        
        std::vector< std::pair<uint32_t, uint32_t> >* all_cor
        );



uint32_t
ransac_core(
        vcl_vector< ellipse > &ellipses1,
        vcl_vector< ellipse > &ellipses2,
        vector< pair<uint32_t, uint32_t> > &putativeMatches,
        
        int max_cor,
        float error_thresh,
        float low_area_change,
        float high_area_change,
        int max_reest,
        
        float* best_h,
        vector< pair<uint32_t,uint32_t> >& best_cor
        );

#endif //_RANSAC_RELJA_RETRIEVAL_H_
