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

#include "ransac_relja_retrieval.h"

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
        ){
    
    // decimate putative matches
    
    
    // If either of the sizes is zero, we need to exit cleanly.
    uint32_t size1= ellipses1.size(), size2= ellipses2.size();
    if (size1 == 0 || size2 == 0) {
        std::fill(best_h, best_h + 9, 0.0f);
        best_cor.clear();
        return 0;
    }
    
    // get putative matches
    vcl_vector< vcl_pair<uint32_t, uint32_t> > putativeMatches;
    putative_quantized::getPutativeMatches( ids1, ids2, putativeMatches );
    
    return
    ransac_core( ellipses1, ellipses2, putativeMatches,
                 max_cor, error_thresh, low_area_change, high_area_change, max_reest,
                 best_h, best_cor );
    
}



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
        ){
    
    // If either of the sizes is zero, we need to exit cleanly.
    uint32_t size1= ellipses1.size(), size2= ellipses2.size();
    if (size1 == 0 || size2 == 0) {
        std::fill(best_h, best_h + 9, 0.0f);
        best_cor.clear();
        return 0;
    }
    
    // get putative matches
    vcl_vector< vcl_pair<uint32_t, uint32_t> > putativeMatches;
    putative_desc<DescType>::getPutativeMatches( desc1, ellipses1.size(), desc2, ellipses2.size(), ndims, putativeMatches, use_lowe, deltaSq, epsilon );

    if (all_cor) {
        *all_cor = putativeMatches;
    }
  
    uint32_t score= 
    ransac_core( ellipses1, ellipses2,
                 putativeMatches,
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
        ){
    
    // extract information about ellipses
        
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
            error_thresh, low_area_change, high_area_change,
            verbose );
        fromF= false;
    #else
        model_fitter *fitterObj= NULL;
            #ifdef _DO_FAF4_
                fitterObj=new fit_fund_affine4(
                    ellipses1, ellipses2,
                    putativeMatches,
                    error_thresh, _AB_RATIO_LIMIT_,
                    verbose );
            #else
                fitterObj= new fit_fund_affine2(
                    ellipses1, ellipses2,
                    putativeMatches,
                    error_thresh, _AB_RATIO_LIMIT_,
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
        error_thresh, low_area_change, high_area_change,
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
                error_thresh, _AB_RATIO_LIMIT_,
                verbose );
        #else
            fitterObj=  new fit_fund_affine2(
                ellipses1, ellipses2,
                putativeMatches,
                error_thresh, _AB_RATIO_LIMIT_,
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
            
      best_cor.clear();
      best_cor.swap( fitterObj->bestInliers );
      /*
      // was before swap, if to reactivate write in terms of iterators
      best_cor.reserve( fitterObj->bestNInliers );
      for (uint32_t i=0; i<fitterObj->bestNInliers; i++){
          best_cor.push_back( make_pair( fitterObj->bestInliers[i].first, fitterObj->bestInliers[i].second ) );
      }
      */
            
  }
  
  delete fitterObj;
  
  return score;
}
