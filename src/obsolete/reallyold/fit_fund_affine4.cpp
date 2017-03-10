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

#include "fit_fund_affine4.h"

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

#include <math.h> //temp for isnan

fit_fund_affine4::fit_fund_affine4(
                vcl_vector< ellipse > aEllipses1, vcl_vector< ellipse > aEllipses2,
                vcl_vector< vcl_pair<uint32_t, uint32_t> > &aPutativeMatches,
                double aErrorThres, double abRatioLimit,
                bool aVerbose ){
    
    verbose= aVerbose;
    
    ellipses1= aEllipses1;
    ellipses2= aEllipses2;
    putativeMatches= aPutativeMatches;
    
    // filter out putative matches where change in scale is >5
    uint32_t pID1, pID2;
    vgl_homg_point_2d<double> p1, p2;
    
    double areaChange;
    
    for ( size_t iPutative = 0; iPutative < putativeMatches.size(); iPutative++ ){
        pID1= putativeMatches[ iPutative ].first;
        pID2= putativeMatches[ iPutative ].second;
        ellipses1[ pID1 ].getCentre( p1 );
        ellipses2[ pID2 ].getCentre( p2 );
        
        areaChange = vcl_sqrt( ellipses2[ pID2 ].getPropAreaSq() / ellipses1[ pID1 ].getPropAreaSq() );
        if ( areaChange < 0.2 || areaChange > 5 ){
            putativeMatches.erase( putativeMatches.begin()+iPutative );
            iPutative--; // so that iPutative be considered again in the next go though the loop
        }
    }
    
    
    // should maybe use memset but probably not worth it
    uint32_t maxpID1= 0, maxpID2= 0;
    for (size_t i_=0; i_< putativeMatches.size(); i_++){
        pID1= putativeMatches[ i_ ].first;
        pID2= putativeMatches[ i_ ].second;
        if (pID1>maxpID1){ maxpID1= pID1; }
        if (pID2>maxpID2){ maxpID2= pID2; }
    }
    point1Used.reserve( maxpID1+1 );
    point2Used.reserve( maxpID2+1 );
    for (uint32_t i_=0; i_<=maxpID1; i_++){ point1Used[i_]= 0; }
    for (uint32_t i_=0; i_<=maxpID2; i_++){ point2Used[i_]= 0; }
    
    
    bestNInliers= 0;
    nFindInliers= 0;
    
    errorThres= aErrorThres;
    abRatioLimitSq= vcl_pow( abRatioLimit , 2 );
}


fit_fund_affine4::~fit_fund_affine4(){
}


uint32_t
fit_fund_affine4::findInliers(
        vnl_matrix_fixed<double,3,3> &F,
        vcl_vector< vcl_pair<uint32_t, uint32_t> > &inliers, bool returnInliers ){
        
    uint32_t nInliers= 0;
    if (returnInliers)
        inliers.clear();
    
    // normalize F
    F /= F.frobenius_norm(); if (F[2][2]<0) { F= -F; }
    
    // check if a/b within is within limits (if far away from 1 then likely that a line in one image is matched with all the lines in the other image)
    double aSq= F[2][0]*F[2][0] + F[2][1]*F[2][1];
    double bSq= F[0][2]*F[0][2] + F[1][2]*F[1][2];
    
    if ( aSq > bSq * abRatioLimitSq || bSq > aSq * abRatioLimitSq ){
        nInliers= 0;
        return nInliers;
    }
    
    double sinAlpha, cosAlpha, sinBeta, cosBeta;
    sinAlpha= -F[2][0] / vcl_sqrt(aSq); cosAlpha= F[2][1] / vcl_sqrt(aSq);
    sinBeta = -F[0][2] / vcl_sqrt(bSq); cosBeta = F[1][2] / vcl_sqrt(bSq);
    
    uint32_t pID1, pID2;
    
    vgl_homg_point_2d<double> p1, p2;
    
    double error, denom;
    #ifdef _TAN_CHANGE_
        double distChangeSq;
    #endif
    denom = vcl_sqrt( 1 - F[2][2]*F[2][2] );  // = sqrt(a^2+b^2+c^2+d^2), using the fact that we already normalized F so that F(:)'*F(:)=1
    
    vnl_vector<double> p1_vec(3), p2_vec(3);
    
    nFindInliers++;
    
    for ( size_t iPutative=0; iPutative < putativeMatches.size(); iPutative++ ){
        pID1= putativeMatches[ iPutative ].first;
        pID2= putativeMatches[ iPutative ].second;
        if (point1Used[ pID1 ]==nFindInliers || point2Used[ pID2 ]==nFindInliers)
            continue;
        ellipses1[ pID1 ].getCentre( p1 );
        ellipses2[ pID2 ].getCentre( p2 );
        
        p1_vec[0]= p1.x(); p1_vec[1]= p1.y(); p1_vec[2]= 1.0;
        p2_vec[0]= p2.x(); p2_vec[1]= p2.y(); p2_vec[2]= 1.0;
        error= vcl_abs( dot_product( p2_vec, F * p1_vec ) / denom ); // p2' * F * p1 / sqrt(a^2+b^2+c^2+d^2)
        
        if ( error < errorThres ){
#ifdef _TAN_CHANGE_
            // | d'/d |=| a/b |
            distChangeSq=
                ( aSq * vcl_pow( ellipses1[ pID1 ].getDistBetweenTangents( cosAlpha, sinAlpha ) , 2 ) ) /
                ( bSq * vcl_pow( ellipses2[ pID2 ].getDistBetweenTangents( cosBeta , sinBeta  ) , 2 ) );
            if ( distChangeSq < _TAN_CHANGE_THRESH_*_TAN_CHANGE_THRESH_ &&
                 1.0/distChangeSq < _TAN_CHANGE_THRESH_*_TAN_CHANGE_THRESH_ ){
#endif
                nInliers++;
                point1Used[ pID1 ]= nFindInliers;
                point2Used[ pID2 ]= nFindInliers;
                if (returnInliers)
                    inliers.push_back( vcl_pair<uint32_t, uint32_t>( pID1, pID2 ) );
#ifdef _TAN_CHANGE_
            }
#endif
        }
        
    }
    
    return nInliers;
    
}


uint32_t
fit_fund_affine4::fit( vcl_vector< vcl_pair<uint32_t, uint32_t> > &matches, bool aFinalFit ){
    
    // Gold Standard from Multiple View Geometry
    
    // probably better to pre-normalize? !!
    
    uint32_t nMatches= matches.size();
    
    uint32_t pID1, pID2;
    vgl_homg_point_2d<double> p1, p2;
    
    double x1bar=0, x2bar=0, y1bar=0, y2bar=0;
    
    for ( size_t iMatch=0; iMatch < nMatches; iMatch++ ){
        pID1= matches[ iMatch ].first;
        pID2= matches[ iMatch ].second;
        // assumes that getCentre returns p.w()==1
        ellipses1[ pID1 ].getCentre( p1 );
        ellipses2[ pID2 ].getCentre( p2 );
        x1bar+= p1.x();
        y1bar+= p1.y();
        x2bar+= p2.x();
        y2bar+= p2.y();
    }
    
    x1bar /= nMatches; y1bar /= nMatches;
    x2bar /= nMatches; y2bar /= nMatches;
    
    vnl_matrix<double> A( nMatches , 4 );
    
    for ( size_t iMatch=0; iMatch < nMatches; iMatch++ ){
        pID1= matches[ iMatch ].first;
        pID2= matches[ iMatch ].second;
        // assumes that getCentre returns p.w()==1
        ellipses1[ pID1 ].getCentre( p1 );
        ellipses2[ pID2 ].getCentre( p2 );
        A[ iMatch ][ 0 ]= p2.x() - x2bar;
        A[ iMatch ][ 1 ]= p2.y() - y2bar;
        A[ iMatch ][ 2 ]= p1.x() - x1bar;
        A[ iMatch ][ 3 ]= p1.y() - y1bar;
    }
    
    vnl_svd_economy<double> svd(A);
    vnl_vector<double> fvector = svd.nullvector();
    
    vnl_matrix_fixed<double,3,3> F;
    
    F[0][0]= 0.0; F[0][1]= 0.0; F[0][2]= fvector[0];
    F[1][0]= 0.0; F[1][1]= 0.0; F[1][2]= fvector[1];
    F[2][0]= fvector[2];
    F[2][1]= fvector[3];
    F[2][2]= -( fvector[0]*x2bar + fvector[1]*y2bar + fvector[2]*x1bar + fvector[3]*y1bar );
        
    uint32_t nInliers;
    if ( aFinalFit ){
        bestF= F;
        nInliers= findInliers( bestF, bestInliers, true ); // or first check if better?
        bestNInliers= nInliers;
    } else {
        vcl_vector< vcl_pair<uint32_t, uint32_t> > inliers;
        nInliers= findInliers( F, inliers, false );
        if (nInliers > bestNInliers){
            bestNInliers= nInliers;
            bestF= F;
        }
    }
    
    //vcl_cout<<"\t\t\t--- nInliers= "<<nInliers<<" bestNInliers= "<<bestNInliers<<"\n";
    return nInliers;
}

uint32_t
fit_fund_affine4::fitMinimalModel( vcl_vector< vcl_pair<uint32_t, uint32_t> > &samples ){
    return fit( samples, false );
}


uint32_t
fit_fund_affine4::fitFinalModel( uint32_t nReest ){
    
    if (verbose){
        vcl_printf("\nNumber of inliers: %d\n", bestNInliers);
        vcl_printf("Best model before optimization:\n");
        vcl_cout << bestF <<"\n";
    }
    
    if ( bestInliers.size() != bestNInliers )
        findInliers( bestF, bestInliers, true );
    
    uint32_t bestNInliers_old= bestNInliers;
    vnl_matrix_fixed<double,3,3> bestF_old= bestF;
    vcl_vector< vcl_pair<uint32_t, uint32_t> > bestInliers_old;
    bestInliers_old.reserve( bestNInliers_old );
    for (uint32_t iInliers= 0; iInliers < bestNInliers; iInliers++)
        bestInliers_old.push_back(
            vcl_pair<uint32_t, uint32_t>( bestInliers[iInliers].first, bestInliers[iInliers].second )
        );
    
    for (uint32_t iReest=0; iReest<nReest; iReest++){
        
        if ( bestNInliers < getNumberToFinalFit() )
            break;
        
        fit( bestInliers, true );
    }
    
    // check if optimization improved things
    if (bestNInliers_old > bestNInliers){
        bestNInliers= bestNInliers_old;
        bestInliers_old.swap( bestInliers );
        bestF= bestF_old;
    }
    
    if (verbose){
        vcl_printf("\nNumber of inliers: %d\n", bestNInliers);
        vcl_printf("Best model after optimization:\n");
        vcl_cout << bestF <<"\n";
    }
        
    return bestNInliers;
}
