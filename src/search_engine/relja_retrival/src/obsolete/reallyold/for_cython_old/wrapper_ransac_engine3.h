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

#ifndef _RELJA_WRAPPER_RANSAC_ENGINE3_H_
#define _RELJA_WRAPPER_RANSAC_ENGINE3_H_

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

// copied stuff from James's code (jp_ransac2.hpp) in order to avoid further Cython complications

// careful about error_thresh, low_area_change, high_area_change since James defined them as squares of those quantities!

//HACK!!
//void
uint32_t
ransac(
       const char* data1,
       uint32_t size1,
  
       const char* data2,
       uint32_t size2,

       int stride,

       int id_off,
       int x_off,
       int y_off,
       int a_off,
       int b_off,
       int c_off,
       int theta_off,
       
       int max_cor,
       float error_thresh,
       float low_area_change,
       float high_area_change,
       int max_reest,

       float* best_h,
       std::vector< std::pair<uint32_t,uint32_t> >& best_cor
       );



template<class DescType>
//void
uint32_t
ransac_desc(
       const char* data1,
       uint32_t size1,

       const char* data2,
       uint32_t size2,

       int stride,

       int x_off,
       int y_off,
       int a_off,
       int b_off,
       int c_off,
       int theta_off,

       const DescType* desc1,
       const DescType* desc2,
       uint32_t ndims,

       float error_thresh,
       float low_area_change,
       float high_area_change,
       int max_reest,
       float epsilon, // Used for distance threshold test.
       float delta, // Used for Lowe's second NN test.
       int use_lowe, // Select Lowe method (=1) or distance threshold (=0)

       float* best_h,
       std::vector< std::pair<uint32_t, uint32_t> >& best_cor,
       float& weighted_inliers,

       std::vector< std::pair<uint32_t, uint32_t> >* all_cor
       );



//void
uint32_t
ransac_core(
       const char *data1,
       const char *data2,
       uint32_t nEllipses1, uint32_t nEllipses2,
       std::vector< std::pair<uint32_t, uint32_t> > &putativeMatches,
       
       int stride,
       
       int x_off,
       int y_off,
       int a_off,
       int b_off,
       int c_off,
       
       int max_cor,
       float error_thresh,
       float low_area_change,
       float high_area_change,
       int max_reest,
       
       float* best_h,
       std::vector< std::pair<uint32_t,uint32_t> >& best_cor
       );

#endif //_RELJA_WRAPPER_RANSAC_ENGINE3_H_