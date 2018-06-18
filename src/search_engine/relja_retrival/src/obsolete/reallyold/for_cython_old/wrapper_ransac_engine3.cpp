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

#include "wrapper_ransac_engine3.h"

#include "ransac.h"
#include "fit_affine.h"
#include "fit_fund_affine4.h"
#include "fit_fund_affine2.h"
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

// copied stuff from James's code (jp_ransac2.cpp) in order to avoid further Cython complications

// careful about error_thresh, low_area_change, high_area_change since James defined them as squares of those quantities!

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
       vector< pair<uint32_t,uint32_t> >& best_cor
       ){
    
    
    // If either of the sizes is zero, we need to exit cleanly.
    if (size1 == 0 || size2 == 0) {
        std::fill(best_h, best_h + 9, 0.0f);
        best_cor.clear();
        //return;
        return 0;
    }
    
    // extract ids
    vcl_vector<uint32_t> ids1;
    ids1.reserve( size1 );
    for (uint32_t i=0; i<size1; i++) {
        ids1.push_back( *(uint32_t*)(data1 + stride*i + id_off) );
    }
    
    vcl_vector<uint32_t> ids2;
    ids2.reserve( size2 );
    for (uint32_t i=0; i<size2; i++) {
        ids2.push_back( *(uint32_t*)(data2 + stride*i + id_off) );
    }
    
    // get putative matches
    vcl_vector< vcl_pair<uint32_t, uint32_t> > putativeMatches;
    putative_quantized::getPutativeMatches( ids1, ids2, putativeMatches );
    
    return
    ransac_core( data1, data2, ids1.size(), ids2.size(),
                 putativeMatches,
                 stride,
                 x_off, y_off, a_off, b_off, c_off,
                 max_cor, error_thresh, low_area_change, high_area_change, max_reest,
                 best_h, best_cor );
    
}



template<class DescType>
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
       vector< pair<uint32_t, uint32_t> >& best_cor,
       float& weighted_inliers,

       vector< pair<uint32_t, uint32_t> >* all_cor // If this is non-zero, we return all the putative correspondences
       ){
    
    // If either of the sizes is zero, we need to exit cleanly.
    if (size1 == 0 || size2 == 0) {
        std::fill(best_h, best_h + 9, 0.0f);
        best_cor.clear();
        //return;
        return 0;
    }
    
    // 1. Find putative matches.
    vector< pair<uint32_t,uint32_t> > cor;

    if (!use_lowe) { // Use the epsilon measure.
      for (uint32_t i=0; i<size1; ++i) {
        for (uint32_t j=0; j<size2; ++j) {
          float dsq = jp_dist_l2(&desc1[i*ndims], &desc2[j*ndims], ndims);
          if (dsq < (epsilon*epsilon)) cor.push_back(make_pair(i, j));
        }
      }
    }
    else {
      static const int numnn = 2;
      pair<uint32_t,float> nns[numnn+1];
      for (uint32_t i=0; i<size1; ++i) {
        nns[0] = nns[1] = nns[2] = make_pair(-1, std::numeric_limits<float>::max());
        for (uint32_t j=0; j<size2; ++j) {
          // One potential improvement is to copy desc1 into an aligned
          // buffer...
          float dsq = jp_dist_l2(&desc1[i*ndims], &desc2[j*ndims], ndims);
      
          if (dsq < nns[numnn-1].second) {
            int k;
            for (k=numnn; k>0 && nns[k-1].second > dsq; --k)
              nns[k] = nns[k-1];
            nns[k] = make_pair(j, dsq);
          }
        }
        assert(nns[0].second <= nns[1].second);
        if ((nns[0].second/nns[1].second) < delta) {
          cor.push_back(make_pair(i, nns[0].first));
        }
      }
    }

    if (all_cor) {
        *all_cor = cor;
    }
  
    uint32_t score= 
    ransac_core( data1, data2, size1, size2,
                 cor,
                 stride,
                 x_off, y_off, a_off, b_off, c_off,
                 0, error_thresh, low_area_change, high_area_change, max_reest,
                 best_h, best_cor );
    
  
    return score;
}



#ifdef _NEIGH_

const uint32_t nNeigh= 3;

void insert_neigh( uint32_t &nN, double dsSq[], double dSq, uint32_t neighs[], uint32_t idnew, uint32_t nNeigh){
        
    if (nN < nNeigh){
        dsSq[ nN ]= dSq;
        neighs[ nN ]= idnew;
        nN++;
    } else {
        if (dsSq[ nN-1 ]>dSq){
            dsSq[ nN-1 ]= dSq;
            neighs[ nN-1 ]= idnew;
        }
    }
    
    for (uint32_t i= nN-1; (i>0) && (dsSq[i-1] > dsSq[i]) ; i--){
        double d_temp= dsSq[i];
        dsSq[i]= dsSq[i-1];
        dsSq[i-1]= d_temp;
        uint32_t id_temp= neighs[i];
        neighs[i]= neighs[i-1];
        neighs[i-1]= id_temp;
    }
    
}

void removeExtraNeigh(
    double x[], double y[],
    uint32_t nN[], double dsSq[][nNeigh], uint32_t neighs[][nNeigh], uint32_t nIn
    ){
    
    double dSq;
    
    for (uint32_t i=0; i<nIn; i++){
        while( nN[i] > 0 && nN[i]>nNeigh ){
            uint32_t i_= neighs[i][ nN[i]-1 ];
            dSq= (x[i]-x[i_])*(x[i]-x[i_]) + (y[i]-y[i_])*(y[i]-y[i_]);
            if (dSq < 1.1*1.1 * dsSq[i][ nN[i]-1 ])
                break;
            nN[i]--;
        }
    }
}

void neighbourConsensus(
        model_fitter *fitterObj,
        vcl_vector< ellipse > &ellipses1, vcl_vector< ellipse > &ellipses2,
        vector< pair<uint32_t, uint32_t> > &putativeMatches,
        uint32_t &score ) {
    const uint32_t maxToCheck= 300; // not even 50 is enough: see pompidou_1 in Paris
    uint32_t prevBestN= 0;
    
    do{
        prevBestN= fitterObj->bestNInliers;

        if ( fitterObj->bestNInliers < maxToCheck ){
            uint32_t nIn= fitterObj->bestNInliers;
            uint32_t neighs1[maxToCheck][nNeigh], neighs2[maxToCheck][nNeigh];
            uint32_t nN1[maxToCheck], nN2[maxToCheck];
            double ds1Sq[maxToCheck][nNeigh], ds2Sq[maxToCheck][nNeigh], dSq1, dSq2;
            double x1[maxToCheck], y1[maxToCheck], x2[maxToCheck], y2[maxToCheck];
            // init
            for (uint32_t i=0; i<nIn; i++){
                nN1[i]=0;
                nN2[i]=0;
                x1[i]= ellipses1[fitterObj->bestInliers[i].first].x;
                y1[i]= ellipses1[fitterObj->bestInliers[i].first].y;
                x2[i]= ellipses2[fitterObj->bestInliers[i].second].x;
                y2[i]= ellipses2[fitterObj->bestInliers[i].second].y;
            }
            // find neighbours
            for (uint32_t i=0; i<nIn; i++){
                for (uint32_t j=0; j<nIn; j++)
                    if (i!=j){
                        dSq1= (x1[i]-x1[j])*(x1[i]-x1[j]) + (y1[i]-y1[j])*(y1[i]-y1[j]);
                        dSq2= (x2[i]-x2[j])*(x2[i]-x2[j]) + (y2[i]-y2[j])*(y2[i]-y2[j]);
                        insert_neigh( nN1[i], ds1Sq[i], dSq1, neighs1[i], j, nNeigh);
                        insert_neigh( nN2[i], ds2Sq[i], dSq2, neighs2[i], j, nNeigh);
                    }
            }
            removeExtraNeigh(x1, y1, nN1, ds1Sq, neighs1, nIn);
            // threshold on agreeing neighbours
            uint32_t realid=0;
            double propAgree=0;
            for ( size_t i= 0; i < fitterObj->bestInliers.size(); i++ ){
                propAgree= 0;
                for (uint32_t neighI1=0; neighI1 < nN1[realid]; neighI1++)
                    for (uint32_t neighI2=0; neighI2 < nN2[realid]; neighI2++)
                        if (neighs1[realid][neighI1]==neighs2[realid][neighI2]){
                            propAgree++;
                            break;
                        }
                if (nN1[realid]==0 && nN2[realid]==0){
                    propAgree= 1;
                } else {
                    if (nN1[realid]>nN2[realid])
                        propAgree= propAgree/nN1[realid];
                    else
                        propAgree= propAgree/nN2[realid];
                }
                if (propAgree<0.6){
                    fitterObj->bestInliers.erase( fitterObj->bestInliers.begin() + i );
                    i--; // so that i be considered again in the next go though the loop
                }
                realid= realid+1;
            }
            fitterObj->bestNInliers= fitterObj->bestInliers.size();
            
            #ifdef _DO_HAFF_THEN_FAF2_
                score= fitterObj->bestNInliers;
            #endif
        }


    } while( true && prevBestN != fitterObj->bestNInliers );
}
#endif


//void
uint32_t
ransac_core(
       const char *data1,
       const char *data2,
       uint32_t nEllipses1, uint32_t nEllipses2,
       vector< pair<uint32_t, uint32_t> > &putativeMatches,
       
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
       vector< pair<uint32_t,uint32_t> >& best_cor
       ){

    // decimate putative matches
    
    
    // extract information about ellipses
    vcl_vector< ellipse > ellipses1, ellipses2;
    ellipses1.reserve( nEllipses1 );
    ellipses2.reserve( nEllipses2 );
    
    
    float *x, *y, *a, *b, *c;
    x= (float*)&data1[x_off];
    y= (float*)&data1[y_off];
    a= (float*)&data1[a_off];
    b= (float*)&data1[b_off];
    c= (float*)&data1[c_off];
    
    if (stride % 4) {
        vcl_printf("stride mod 4 !=0 !!!\n"); // from James's code
        exit(-1);
      }
    stride/=4;
    
    for (uint32_t i=0; i<nEllipses1; i++) {
        ellipses1.push_back( ellipse(
            x[ stride * i ],
            y[ stride * i ],
            a[ stride * i ],
            b[ stride * i ],
            c[ stride * i ]
        ) );
    }
    
    x= (float*)&data2[x_off];
    y= (float*)&data2[y_off];
    a= (float*)&data2[a_off];
    b= (float*)&data2[b_off];
    c= (float*)&data2[c_off];
    
    for (uint32_t i=0; i<nEllipses2; i++) {
        ellipses2.push_back( ellipse(
            x[ stride * i ],
            y[ stride * i ],
            a[ stride * i ],
            b[ stride * i ],
            c[ stride * i ]
        ) );
    }
    
    bool debug_this= false;
//     vcl_cout << "\n putm= \n" << putativeMatches.size() << "\n";
//     if (putativeMatches.size() == 441) {
    if (putativeMatches.size() == 1733) {
        debug_this= true;
        vcl_cout << "nel1= " << ellipses1.size() << ";\n";
        vcl_cout << "nel2= " << ellipses2.size() << ";\n";
        ellipse *el;
        vcl_cout << "el1= [\n";
        for (uint32_t i=0; i<ellipses1.size(); i++){
            el= &ellipses1[i];
            vcl_cout << el->x <<" "<< el->y <<" "<< el->a <<" "<< el->b <<" "<< el->c <<"; \n";
        }
        vcl_cout << "];\n";
        vcl_cout << "el2= [\n";
        for (uint32_t i=0; i<ellipses2.size(); i++){
            el= &ellipses2[i];
            vcl_cout << el->x <<" "<< el->y <<" "<< el->a <<" "<< el->b <<" "<< el->c <<"; \n";
        }
        vcl_cout << "];\n";
        
        vcl_cout << "nputm= " << putativeMatches.size() << ";\n";
        vcl_cout << "putm= [\n";
        for (uint32_t i=0; i<putativeMatches.size(); i++){
            vcl_cout << putativeMatches[i].first << " " << putativeMatches[i].second<<"; \n";
        }
        vcl_cout << "];\n";
    }
    
    bool verbose= false;
#ifdef _RANSAC_VERBOSE_
    verbose= true;
#endif

    uint32_t score= 0;
    
    bool fromF= true;
    
    // setup model_fitter
    
#ifndef _DO_HAFF_THEN_FAF2_
    
    #ifdef _DO_HAFF_
        fit_affine *fitterObj= new fit_affine(
            ellipses1, ellipses2,
            putativeMatches,
            vcl_sqrt(error_thresh), vcl_sqrt(low_area_change), vcl_sqrt(high_area_change),
            verbose );
        fromF= false;
    #else
        model_fitter *fitterObj= NULL;
            #ifdef _DO_FAF4_
                fitterObj=new fit_fund_affine4(
                    ellipses1, ellipses2,
                    putativeMatches,
                    vcl_sqrt(error_thresh), _AB_RATIO_LIMIT_,
                    verbose );
            #else
                fitterObj= new fit_fund_affine2(
                    ellipses1, ellipses2,
                    putativeMatches,
                    vcl_sqrt(error_thresh), _AB_RATIO_LIMIT_,
                    verbose );
            #endif
        fromF= true;
    #endif
        
    // run RANSAC
    bool success= ransac::doRansac( putativeMatches, *fitterObj, max_reest, 0, verbose );
    
    #ifdef _DO_HAFF_
        if ( success ){
            ((fit_affine*)fitterObj)->bestH->exportToFloatArray( best_h );
            score= fitterObj->bestNInliers;
        } else {
            std::fill(best_h, best_h + 9, 0.0f); best_h[0]=1.0; best_h[4]=1.0; best_h[8]=1.0;
            score= 0;
        }
    #endif
    
    
#else //#ifndef _DO_HAFF_THEN_FAF2_

    model_fitter *fitterObj= new fit_affine(
        ellipses1, ellipses2,
        putativeMatches,
        vcl_sqrt(error_thresh), vcl_sqrt(low_area_change), vcl_sqrt(high_area_change),
        verbose );
    fromF= false;

    bool success= ransac::doRansac( putativeMatches, *fitterObj, max_reest, 0, verbose );
    if ( fitterObj->bestNInliers <= _HAFF_MIN_INLIERS_ ){
        success= false;
    }

    if ( success ){
        ((fit_affine*)fitterObj)->bestH->exportToFloatArray( best_h );
        score= (fitterObj->bestNInliers)*1000;
    } else {
        delete fitterObj;
        #ifdef _DO_FAF4_
            fitterObj=  new fit_fund_affine4(
                ellipses1, ellipses2,
                putativeMatches,
                vcl_sqrt(error_thresh), _AB_RATIO_LIMIT_,
                verbose );
        #else
            fitterObj=  new fit_fund_affine2(
                ellipses1, ellipses2,
                putativeMatches,
                vcl_sqrt(error_thresh), _AB_RATIO_LIMIT_,
                verbose );
        #endif
        success= ransac::doRansac( putativeMatches, *fitterObj, max_reest, 0, verbose );
        std::fill(best_h, best_h + 9, 0.0f); best_h[0]=1.0; best_h[4]=1.0; best_h[8]=1.0;
        if (success){
            score= (fitterObj->bestNInliers);
        }
        fromF= true;
    }
    
#endif //#else ifndef _DO_HAFF_THEN_FAF2_


#ifndef _DO_HAFF_

    uint32_t totalNBefore= fitterObj->bestNInliers;

    #ifdef _POST_TAN_CHANGE_
        // needs to be before looseH since that deletes fitterObj!!
        {
            const uint32_t maxToCheck= 300; // not even 50 is enough: see pompidou_1 in Paris
            
            if ( success && fromF && fitterObj->bestNInliers < maxToCheck ){
            
                vnl_matrix_fixed<double,3,3> F=  ((fit_fund_affine2*)fitterObj)->bestF;
                F /= F.frobenius_norm(); if (F[2][2]<0) { F= -F; }
                double aSq= F[2][0]*F[2][0] + F[2][1]*F[2][1];
                double bSq= F[0][2]*F[0][2] + F[1][2]*F[1][2];
                double sinAlpha, cosAlpha, sinBeta, cosBeta;
                sinAlpha= -F[2][0] / vcl_sqrt(aSq); cosAlpha= F[2][1] / vcl_sqrt(aSq);
                sinBeta = -F[0][2] / vcl_sqrt(bSq); cosBeta = F[1][2] / vcl_sqrt(bSq);

                uint32_t realid=0;
                double distChangeSq;
                for (uint32_t i=0; i<fitterObj->bestInliers.size(); i++){
                    distChangeSq=
                        ( aSq * vcl_pow( ellipses1[ fitterObj->bestInliers[i].first ].getDistBetweenTangents( cosAlpha, sinAlpha ) , 2 ) ) /
                        ( bSq * vcl_pow( ellipses2[ fitterObj->bestInliers[i].second ].getDistBetweenTangents( cosBeta , sinBeta  ) , 2 ) );
                    if ( distChangeSq >= _POST_TAN_CHANGE_THRESH_*_POST_TAN_CHANGE_THRESH_ ||
                         1.0/distChangeSq >= _POST_TAN_CHANGE_THRESH_*_POST_TAN_CHANGE_THRESH_ ){
                         fitterObj->bestInliers.erase( fitterObj->bestInliers.begin() + i );
                         i--; // so that i be considered again in the next go though the loop
                    }
                    realid= realid+1;
                }
                fitterObj->bestNInliers= fitterObj->bestInliers.size();
                
                #ifdef _DO_HAFF_THEN_FAF2_
                    score= fitterObj->bestNInliers;
                #endif //#ifdef _DO_HAFF_THEN_FAF2_

            }
        }
    #endif //#ifdef _POST_TAN_CHANGE_


    #ifdef _LOOSEH_AFTER_
        
        if (success && fromF){
            putativeMatches.clear();
            putativeMatches.reserve( fitterObj->bestNInliers );
            for (uint32_t i=0; i<fitterObj->bestNInliers; i++){
                putativeMatches.push_back( make_pair( fitterObj->bestInliers[i].first, fitterObj->bestInliers[i].second ) );
            }
            
            delete fitterObj;
            
            fitterObj= new fit_affine(
                ellipses1, ellipses2,
                putativeMatches,
                200.0, 0.2, 5.0,
                verbose );        

            vector< pair<uint32_t, uint32_t> > putativeMatches_new;
            success= ransac::doRansac( putativeMatches, *fitterObj, max_reest, 0, verbose );
            
            score= (fitterObj->bestNInliers);
        }

    #endif //#ifdef _LOOSEH_AFTER_



    #ifdef _NEIGH_
        if (success && fromF)
            neighbourConsensus(fitterObj, ellipses1, ellipses2, putativeMatches, score);
    #endif //#ifdef _NEIGH_


    #ifdef _SURVIVAL_

        double ratio= double(fitterObj->bestNInliers) / double(totalNBefore);
        if (ratio < _SURVIVAL_THRESH_){
            fitterObj->bestNInliers= 0;
            fitterObj->bestInliers.clear();
            #ifdef _DO_HAFF_THEN_FAF2_
                score= 0;
            #endif //#ifdef _DO_HAFF_THEN_FAF2_
        }
        
    #endif //#ifdef _SURVIVAL_


#endif //#ifndef _DO_HAFF_

  
  if ( !success ){
      std::fill(best_h, best_h + 9, 0.0f);
      best_cor.clear();
  } else {
  
      // !!careful about this - if max_reest <=0 then no inliers will be found if this is still set in fit_homog.cpp
      
#ifndef _DO_HAFF_
      std::fill(best_h, best_h + 9, 0.0f); best_h[0]=1.0; best_h[4]=1.0; best_h[8]=1.0;
#endif
      
      if (debug_this) {
          vcl_cout<<"ninliers= "<< fitterObj->bestNInliers <<";\n";
          vcl_cout << "m= [\n";
      }
      
      best_cor.clear();
      best_cor.reserve( fitterObj->bestNInliers );
      for (uint32_t i=0; i<fitterObj->bestNInliers; i++){
          best_cor.push_back( make_pair( fitterObj->bestInliers[i].first, fitterObj->bestInliers[i].second ) );
          if (debug_this) {
              vcl_cout<< fitterObj->bestInliers[i].first << " " << fitterObj->bestInliers[i].second <<";\n";
          }
      }
      
      if (debug_this) {
          vcl_cout << "];\n";
      }
            
  }
  
  delete fitterObj;
  
  return score;
}





template
uint32_t
ransac_desc<unsigned char>(
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

     const unsigned char* desc1,
     const unsigned char* desc2,
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
     
     std::vector< std::pair<uint32_t, uint32_t> >* all_cor);

template
uint32_t
ransac_desc<float>(
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

       const float* desc1,
       const float* desc2,
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

       std::vector< std::pair<uint32_t, uint32_t> >* all_cor);
