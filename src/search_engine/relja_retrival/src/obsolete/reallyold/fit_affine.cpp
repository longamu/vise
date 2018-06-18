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

#include "fit_affine.h"

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

fit_affine::fit_affine( vcl_vector< ellipse > aEllipses1, vcl_vector< ellipse > aEllipses2,
                vcl_vector< vcl_pair<uint32_t, uint32_t> > &aPutativeMatches,
                double aErrorThres, double aLowAreaChange, double aHighAreaChange,
                bool aVerbose ){
    
    verbose= aVerbose;
    
    ellipses1= aEllipses1;
    ellipses2= aEllipses2;
    putativeMatches= aPutativeMatches;
    
    uint32_t pID1, pID2;
    // should maybe use memset but probably not worth it
    uint32_t maxpID1= 0, maxpID2= 0;
    for (vcl_vector< vcl_pair<uint32_t, uint32_t> >::iterator itP=putativeMatches.begin();
         itP != putativeMatches.end();
         ++itP){
        pID1= itP->first;
        pID2= itP->second;
        if (pID1>maxpID1){ maxpID1= pID1; }
        if (pID2>maxpID2){ maxpID2= pID2; }
    }
    point1Used.resize( maxpID1+1, 0 );
    point2Used.resize( maxpID2+1, 0 );
    
    bestNInliers= 0;
    bestH= new homography();
    nFindInliers= 0;
    
    errorThresSq= vcl_pow( aErrorThres , 2 );
    lowAreaChangeSq= vcl_pow( aLowAreaChange , 2 );
    highAreaChangeSq= vcl_pow( aHighAreaChange , 2 );
}


fit_affine::~fit_affine(){
    delete bestH;
}


uint32_t
fit_affine::findInliers( homography &h, vcl_vector< vcl_pair<uint32_t, uint32_t> > &inliers, bool returnInliers ){
    
    double detASq = vcl_pow( h.getDetAffine(), 2 );
    
    uint32_t nInliers= 0;
    if (returnInliers)
        inliers.clear();
    
    
    uint32_t pID1, pID2;
    
    ++nFindInliers;
    
    vgl_homg_point_2d<double> p1, p2, p, pi;
    if (detASq<1e-4 || isnan(detASq) || vnl_det(h.H.get_matrix())==0){ return 0; }
    vgl_h_matrix_2d<double> Hinv= h.H.get_inverse();
    
    double error, areaChangeSq;
    
    
    for (vcl_vector< vcl_pair<uint32_t, uint32_t> >::iterator itP=putativeMatches.begin();
         itP != putativeMatches.end();
         ++itP){
        pID1= itP->first;
        pID2= itP->second;
        if (point1Used[ pID1 ]==nFindInliers || point2Used[ pID2 ]==nFindInliers)
            continue;
        ellipses1[ pID1 ].getCentre( p1 );
        ellipses2[ pID2 ].getCentre( p2 );
        p= h.H * p1;
        pi= Hinv * p2;
        
        error= vcl_pow( vgl_distance(p1,pi) , 2 ) + vcl_pow( vgl_distance(p2,p), 2 );
        
        if ( error < errorThresSq ){
            areaChangeSq = detASq * ellipses2[ pID2 ].getPropAreaSq() / ellipses1[ pID1 ].getPropAreaSq();
            if ( areaChangeSq > lowAreaChangeSq && areaChangeSq < highAreaChangeSq ){
                ++nInliers;
                point1Used[ pID1 ]= nFindInliers;
                point2Used[ pID2 ]= nFindInliers;
                if (returnInliers)
                    inliers.push_back( vcl_pair<uint32_t, uint32_t>( pID1, pID2 ) );
            }
        }
        
    }
    
    return nInliers;
    
}


uint32_t
fit_affine::fitMinimalModel( vcl_vector< vcl_pair<uint32_t, uint32_t> > &samples ){
    
    homography *h= new homography( ellipses1[ samples[0].first ] , ellipses2[ samples[0].second ] );
    vcl_vector< vcl_pair<uint32_t, uint32_t> > inliers;
    uint32_t nInliers= findInliers( *h, inliers, false );
    
    if ( bestNInliers < nInliers ){
        bestNInliers= nInliers;
        homography *temp; temp= bestH;
        bestH= h;
        h= temp;
        bestInliers.swap( inliers ); // no need if findInliers( .., .., false);
    }
    
    delete h; // now h= either h or ex bestH
    
    return nInliers;
}


void
fit_affine::getHnorm( 
          vcl_vector< vgl_homg_point_2d<double> > &inlierPoints1,
          vcl_vector< vgl_homg_point_2d<double> > &inlierPoints2,
          vgl_norm_trans_2d<double> &normTrans1,
          vgl_norm_trans_2d<double> &normTrans2,
          vnl_matrix_fixed<double,3,3> &Hnorm ){

    // DLT in the case h[2][0]=h[2][1]=0
    
    uint32_t nInliers= inlierPoints1.size();
    
    vnl_matrix<double> A( nInliers*2 , 7 );
    
    // populate A matrix, we want A * Hnorm_vector = 0
    vgl_homg_point_2d<double> p1, p2;
    
    for ( size_t i=0; i < nInliers; ++i ){
        
        p1= normTrans1 * inlierPoints1[i];
        p2= normTrans2 * inlierPoints2[i];
        
        A[ i*2   ][ 0 ]=            0.0;
        A[ i*2   ][ 1 ]=            0.0;
        A[ i*2   ][ 2 ]=            0.0;
        A[ i*2   ][ 3 ]=  p1.x()*p2.w();
        A[ i*2   ][ 4 ]=  p1.y()*p2.w();
        A[ i*2   ][ 5 ]=  p1.w()*p2.w();
        A[ i*2   ][ 6 ]= -p1.w()*p2.y();
        
        A[ i*2+1 ][ 0 ]= -p1.x()*p2.w();
        A[ i*2+1 ][ 1 ]= -p1.y()*p2.w();
        A[ i*2+1 ][ 2 ]= -p1.w()*p2.w();
        A[ i*2+1 ][ 3 ]=            0.0;
        A[ i*2+1 ][ 4 ]=            0.0;
        A[ i*2+1 ][ 5 ]=            0.0;
        A[ i*2+1 ][ 6 ]=  p1.w()*p2.x();
    }
    
    // get Hnorm_vector as the rightmost column of V (i.e. vector with the smallest singular value)
    vnl_svd_economy<double> svd(A);
    vnl_vector<double> Hnorm_vector = svd.nullvector();
    
    Hnorm[0][0]= Hnorm_vector[0]; Hnorm[0][1]= Hnorm_vector[1]; Hnorm[0][2]= Hnorm_vector[2];
    Hnorm[1][0]= Hnorm_vector[3]; Hnorm[1][1]= Hnorm_vector[4]; Hnorm[1][2]= Hnorm_vector[5];
    Hnorm[2][0]=             0.0; Hnorm[2][1]=             0.0; Hnorm[2][2]= Hnorm_vector[6];
    
}



void
fit_affine::getH(
        vcl_vector< vgl_homg_point_2d<double> > &inlierPoints1,
        vcl_vector< vgl_homg_point_2d<double> > &inlierPoints2,
        vgl_h_matrix_2d<double> &H ) {
    
    // normalize points
    vgl_norm_trans_2d<double> normTrans1, normTrans2;
    normTrans1.compute_from_points( inlierPoints1, true);
    normTrans2.compute_from_points( inlierPoints2, true);
    
    // compute
    vnl_matrix_fixed<double,3,3> Hnorm;
    getHnorm( inlierPoints1, inlierPoints2, normTrans1, normTrans2, Hnorm );
    
    // denormalize homography
    vgl_h_matrix_2d<double> normTrans2inv= normTrans2.get_inverse();
    H = normTrans2inv * Hnorm * normTrans1;
    
}



uint32_t
fit_affine::fitFinalModel( uint32_t nReest ){
    
    if (verbose){
        vcl_printf("\nNumber of inliers: %d\n", bestNInliers);
        vcl_printf("Best model before optimization:\n");
        vcl_cout << (bestH->H) <<"\n";
    }
    
    if ( bestInliers.size() != bestNInliers )
        findInliers( *bestH, bestInliers, true );
    
    uint32_t bestNInliers_old= bestNInliers;
    vgl_h_matrix_2d<double> H_old= bestH->H;
    vcl_vector< vcl_pair<uint32_t, uint32_t> > bestInliers_old;
    bestInliers_old.reserve( bestNInliers_old );
    for (vcl_vector< vcl_pair<uint32_t, uint32_t> >::iterator itIn= bestInliers.begin();
         itIn!=bestInliers.end();
         ++itIn)
        bestInliers_old.push_back(
            vcl_pair<uint32_t, uint32_t>( itIn->first, itIn->second )
        );
    
    for (uint32_t iReest=0; iReest<nReest; ++iReest){
    
        if ( bestNInliers < getNumberToFinalFit() )
            break;
        
        // get the centers of ellipses
        vcl_vector< vgl_homg_point_2d<double> > points1, points2;
        ellipse::getCentres( ellipses1, points1 );
        ellipse::getCentres( ellipses2, points2 );
        
        // keep only inliers
        vcl_vector< vgl_homg_point_2d<double> > inlierPoints1, inlierPoints2;
        inlierPoints1.reserve( bestNInliers );
        inlierPoints2.reserve( bestNInliers );
        for ( vcl_vector< vcl_pair<uint32_t, uint32_t> >::iterator itIn= bestInliers.begin();
              itIn!=bestInliers.end();
              ++itIn ){
            inlierPoints1.push_back( points1[ itIn->first ] );
            inlierPoints2.push_back( points2[ itIn->second ] );
        }
        
        getH( inlierPoints1, inlierPoints2, bestH->H );
        
        bestNInliers= findInliers( *bestH, bestInliers, true ); // or first check if better?
    
    }
    
    // check if optimization improved things
    if (bestNInliers_old > bestNInliers){
        bestNInliers= bestNInliers_old;
        bestInliers_old.swap( bestInliers );
        bestH->H= H_old;
    }

    
    if (verbose){
        vcl_printf("\nNumber of inliers: %d\n", bestNInliers);
        vcl_printf("Best model after optimization:\n");
        vcl_cout << (bestH->H) <<"\n";
    }
        
    return bestNInliers;
}
