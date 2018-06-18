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

#include "fit_affine3.h"

#include <math.h>

#include "fit_affine.h"


fitAffine3::fitAffine3( std::vector< vgl_homg_point_2d<double> > &aPoints1, std::vector< vgl_homg_point_2d<double> > &aPoints2, std::vector< std::pair<uint32_t, uint32_t> > &aPutativeMatches, double aErrorThres) : putativeMatches(&aPutativeMatches), points1(&aPoints1), points2(&aPoints2) {
    
    uint32_t pID1, pID2;
    // should maybe use memset but probably not worth it
    uint32_t maxpID1= 0, maxpID2= 0;
    for (vcl_vector< vcl_pair<uint32_t, uint32_t> >::iterator itP=putativeMatches->begin();
         itP != putativeMatches->end();
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
    
    std::cout<<"putM: "<<putativeMatches->size()<<"\n";
    
    errorThresSq= pow( aErrorThres , 2 );
    
}



fitAffine3::~fitAffine3(){
    delete bestH;
}



uint32_t
fitAffine3::fitMinimalModel( std::vector< std::pair<uint32_t, uint32_t> > &samples ){
    
    homography *H= new homography();
    
    std::vector< vgl_homg_point_2d<double> > samplePoints1, samplePoints2;
    samplePoints1.reserve( samples.size() );
    samplePoints2.reserve( samples.size() );
    
    for (std::vector< std::pair<uint32_t, uint32_t> >::iterator itS= samples.begin();
         itS!=samples.end();
         ++itS) {
        
        samplePoints1.push_back( points1->at(itS->first) );
        samplePoints2.push_back( points2->at(itS->second) );
        
    }
    
    fit_affine::getH( samplePoints1, samplePoints2, H->H );
    
    uint32_t nInliers= findInliers( *H );
    
    if ( bestNInliers < nInliers ){
        bestNInliers= nInliers;
        homography *temp; temp= bestH;
        bestH= H;
        H= temp;
    }
    
    delete H; // now H= either H or ex bestH
    
    return nInliers;
    
}



uint32_t
fitAffine3::fitFinalModel( uint32_t nReest ){
    
    std::cout<<"pre final "<<bestNInliers<<"\n";
    
    if ( bestInliers.size() != bestNInliers )
        findInliers( *bestH, true, &bestInliers );
    
    // copy current
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
    
    // optimize
    for (uint32_t iReest=0; iReest<nReest; ++iReest){
    
        if ( bestNInliers < getNumberToFinalFit() )
            break;
        
        // keep only inliers
        vcl_vector< vgl_homg_point_2d<double> > inlierPoints1, inlierPoints2;
        inlierPoints1.reserve( bestNInliers );
        inlierPoints2.reserve( bestNInliers );
        for (std::vector< std::pair<uint32_t, uint32_t> >::iterator itIn= bestInliers.begin();
         itIn!=bestInliers.end();
         ++itIn) {
            
            inlierPoints1.push_back( points1->at(itIn->first) );
            inlierPoints2.push_back( points2->at(itIn->second) );
            
        }
        
        std::cout<<inlierPoints1.size()<<"\n";
        fit_affine::getH( inlierPoints1, inlierPoints2, bestH->H );
        
        bestNInliers= findInliers( *bestH, true, &bestInliers );
    
    }
    
    // check if optimization improved things
    if (bestNInliers_old > bestNInliers){
        bestNInliers= bestNInliers_old;
        bestInliers_old.swap( bestInliers );
        bestH->H= H_old;
    }
    
    std::cout<<"post final "<<bestNInliers<<"\n";
    
    return bestNInliers;
    
}



uint32_t
fitAffine3::findInliers( homography &h, bool returnInliers, std::vector< std::pair<uint32_t, uint32_t> > *inliers ){
    
    double detASq = vcl_pow( h.getDetAffine(), 2 );
    
    uint32_t nInliers= 0;
    if (returnInliers)
        inliers->clear();
    
    
    uint32_t pID1, pID2;
    
    ++nFindInliers;
    
    vgl_homg_point_2d<double> p1, p2, p, pi;
    if (detASq<1e-4 || isnan(detASq) || vnl_det(h.H.get_matrix())==0){ return 0; }
    vgl_h_matrix_2d<double> Hinv= h.H.get_inverse();
    
    double error;
    
    
    for ( std::vector< std::pair<uint32_t, uint32_t> >::iterator itP=putativeMatches->begin();
         itP != putativeMatches->end();
         ++itP) {
        
        pID1= itP->first;
        pID2= itP->second;
        if (point1Used[ pID1 ]==nFindInliers || point2Used[ pID2 ]==nFindInliers)
            continue;
        p1= points1->at(pID1);
        p2= points2->at(pID2);
        p= h.H * p1;
        pi= Hinv * p2;
        
        error= pow( vgl_distance(p1,pi) , 2 ) + pow( vgl_distance(p2,p), 2 );
        
        if ( error < errorThresSq ){
            ++nInliers;
            point1Used[ pID1 ]= nFindInliers;
            point2Used[ pID2 ]= nFindInliers;
            if (returnInliers)
                inliers->push_back( std::make_pair<uint32_t, uint32_t>( pID1, pID2 ) );
        }
        
    }
    
    return nInliers;
    
}
