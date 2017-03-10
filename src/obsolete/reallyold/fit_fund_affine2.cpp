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

#include "fit_fund_affine2.h"

#include "fit_fund_affine4.h"
#include "model_fitter.h"
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


uint32_t
fit_fund_affine2::fitMinimalModel( vcl_vector< vcl_pair<uint32_t, uint32_t> > &samples ){
    
    uint32_t nInliers, sampleBestNInliers=0;
    
    uint32_t elID1    = samples[ 0 ].first;
    uint32_t elID1prim= samples[ 0 ].second;
    uint32_t elID2    = samples[ 1 ].first;
    uint32_t elID2prim= samples[ 1 ].second;
    
    vgl_h_matrix_2d<double> Ha, Haprim;
    ellipse ellipse2t(0.0,0.0,0.0,0.0,0.0), ellipse2tprim(0.0,0.0,0.0,0.0,0.0);
    
    fit_fund_affine2::affineNormalize(
                ellipses1[ elID1 ], ellipses1[ elID2 ],
                ellipse2t,
                Ha );
    
    fit_fund_affine2::affineNormalize(
                ellipses2[ elID1prim ], ellipses2[ elID2prim ],
                ellipse2tprim,
                Haprim );

    // alpha in [-pi/2,pi/2], beta in [0, 2*pi]
    
    // c= cos(alpha)
    // y2*cos(alpha)=y2prim*cos(beta) => cos(beta)= s*cos(alpha)
    double s= ellipse2t.y / ellipse2tprim.y;
    
    // want n(alpha)=[-sin(alpha); cos(alpha)]:
    // n(alpha)'*C2dual(1:2,1:2)*n(alpha) =
    // n(beta)'*C2dualprim(1:2,1:2)*n(beta)
    // each side simplifies to (using a,b,c of C2dual!)
    // a + (c-a) cos^2(alpha) - 2*b*sin(alpha)*cos(alpha)
    double a, b, c, aprim, bprim, cprim;
    ellipse2t.getDual(a,b,c);
    ellipse2tprim.getDual(aprim,bprim,cprim);
    
    // using cos(alpha)=1/sqrt(1+t^2) and same for beta and tprim
    // t^2(a-aprim) + t(-2b) + (c-aprim - s^2(cprim-aprim)) = 2 bprim s^2 sqrt( (1+t^2)/s^2 - 1)
    // use t2 *t^2 + t1 * t + t0 = 2 tsq sqrt(1+t^2)/s^2-1)
    double t2= a-aprim;
    double t1= -2*b;
    double t0= c-aprim - s*s*(cprim-aprim);
    // squaring the equation gives
    // ( t2^2 ) t^4 + ( 2 t2 t1 ) t^3 + ( 2 t2 t0 + 2 t1^2 - 4 bprim^2 s^2 ) t^2 + ( 2 t1 t0 ) t + ( -4 bprim^2 s^2 (1-s^2) ) = 0
    double k4= t2*t2;
    double k3= 2*t2*t1;
    double k2= 2*t2*t0 + t1*t1 - 4*bprim*bprim*s*s;
    double k1= 2*t1*t0;
    double k0= t0*t0 - 4*bprim*bprim*s*s*(1-s*s);
    
    uint8_t numsol, iSol;
    double t[4];
    solve_quartic_methods::solve_quartic_real_unique( k4, k3, k2, k1, k0, t, numsol );
    
    double cosAlpha, sinAlpha, cosBeta, sinBeta; //, alpha, beta;
    vnl_matrix_fixed<double,3,3> F;
    
#ifdef _FAF2_EXTRA_VERBOSE_
    vcl_cout<< ellipses1[ elID1 ].x <<" "<<ellipses1[ elID1 ].y<<" "<<ellipses1[ elID1 ].a<<" "<<ellipses1[ elID1 ].b << " "<< ellipses1[ elID1 ].c << "\n";
    vcl_cout<< ellipses1[ elID2 ].x <<" "<<ellipses1[ elID2 ].y<<" "<<ellipses1[ elID2 ].a<<" "<<ellipses1[ elID2 ].b << " "<< ellipses1[ elID2 ].c << "\n";
    vcl_cout<< ellipses2[ elID1prim ].x <<" "<<ellipses2[ elID1prim ].y<<" "<<ellipses2[ elID1prim ].a<<" "<<ellipses2[ elID1prim ].b << " "<< ellipses2[ elID1prim ].c << "\n";
    vcl_cout<< ellipses2[ elID2prim ].x <<" "<<ellipses2[ elID2prim ].y<<" "<<ellipses2[ elID2prim ].a<<" "<<ellipses2[ elID2prim ].b << " "<< ellipses2[ elID2prim ].c << "\n";
#endif
    
    for (iSol=0; iSol<numsol; iSol++){
        
        cosAlpha= 1.0     / vcl_sqrt( 1.0 + t[iSol]*t[iSol] );
        sinAlpha= t[iSol] / vcl_sqrt( 1.0 + t[iSol]*t[iSol] );
        //alpha= vcl_atan2( sinAlpha, cosAlpha );
        
        cosBeta= s*cosAlpha;
        // eliminate solutions with impossible states of variables:
        // _ abs(cosBeta)>1
        // _ the squared distance is negative
        if (
            ( vcl_abs(cosBeta) > 1 ) ||
            ((a + (c-a)*cosAlpha*cosAlpha - 2*b*vcl_sqrt(1-cosAlpha*cosAlpha)*cosAlpha) < 0)
            ){
            continue;
        }
        
        // t can be of any sign, thus sin(alpha) can have any sign, cos(alpha) is positive, thus alpha in [-pi/2,pi/2] which is fine. cos(beta)=s cos(alpha) fixes the sign of cos(beta), but sin(beta) can be positive or negative, which corresponds to tprim being positive or negative. This information is lost in squaring, so we need to check all our solutions to see if they hold for the non-squared equations, if not then sin(beta) (i.e. beta) should change the sign.
        
        double epsilon= 1e-4;
        sinBeta= vcl_sqrt( 1-cosBeta*cosBeta );
        if (
            vcl_abs(
                    (a + (c-a)*cosAlpha*cosAlpha - 2*b*sinAlpha*cosAlpha) -
                    (aprim + (cprim-aprim)*cosBeta*cosBeta - 2*bprim*sinBeta*cosBeta)
                ) > epsilon
            ) {
            sinBeta = -sinBeta;
        }
        //beta= atan2( sinBeta, cosBeta ); if (beta<0){ beta= 2*vnl_math::pi + beta; }
        
        // make sure the solution is correct
        if (
            vcl_abs(
                    (a + (c-a)*cosAlpha*cosAlpha - 2*b*sinAlpha*cosAlpha) -
                    (aprim + (cprim-aprim)*cosBeta*cosBeta - 2*bprim*sinBeta*cosBeta)
                ) <= epsilon
            ) {
            
            /*
            a=  1/sqrt(2);
            b= -1/sqrt(2);
            F= [0 0 -b*sin(beta);0 0 b*cos(beta); -a*sin(alpha) a*cos(alpha) 0];
            F= H1prim'*F*H1;
            F= F/sqrt(F(:)'*F(:));
            if (F(3,3)<0), F=-F; end
            */
    
            F[0][0]=       0.0; F[0][1]=       0.0; F[0][2]=  sinBeta;
            F[1][0]=       0.0; F[1][1]=       0.0; F[1][2]= -cosBeta;
            F[2][0]= -sinAlpha; F[2][1]=  cosAlpha; F[2][2]=      0.0;
            F= Haprim.get_matrix().transpose() * F * Ha.get_matrix();
            F /= F.frobenius_norm(); if (F[2][2]<0) { F= -F; }
            
            vcl_vector< vcl_pair<uint32_t, uint32_t> > inliers;
            nInliers= findInliers( F, inliers, false );

            if (nInliers > sampleBestNInliers){
                sampleBestNInliers= nInliers;
            }
            if (nInliers > bestNInliers){
                bestNInliers= nInliers;
                bestF= F;
            }
            
#ifdef _FAF2_EXTRA_VERBOSE_
            vcl_cout<< bestNInliers << " " << nInliers;
            vcl_cout<<" F=["<<F[0][0]<<" "<<F[0][1]<<" "<<F[0][2]<<"; "<<F[1][0]<<" "<<F[1][1]<<" "<<F[1][2]<<"; "<<F[2][0]<<" "<<F[2][1]<<" "<<F[2][2] << "];\n";
#endif
            
        }
        
    }
#ifdef _FAF2_EXTRA_VERBOSE_
    vcl_cout<< "\n";
#endif
    
    return sampleBestNInliers;
}


void
fit_fund_affine2::affineNormalize(
                ellipse &ellipse1, ellipse &ellipse2,
                ellipse &ellipse2transf,
                vgl_h_matrix_2d<double> &Haff ){
        
    // after writing down: a*x^2 + 2*b*x*y + c*y^2 =1 and we want x'^2+y'^2=1
    // first translate the centre, then rotate direction to be vertical (get rotation matrix), after that
    // Aff=[A B;C D] such that Aff * [x;y]=[x';y'] and Aff * [0;1] = [0; y'] => B=0 => simple
    
    // translation
    
    vnl_matrix_fixed<double, 3, 3> trans;
    trans[0][0]= 1.0; trans[0][1]= 0.0; trans[0][2]= -ellipse1.x;
    trans[1][0]= 0.0; trans[1][1]= 1.0; trans[1][2]= -ellipse1.y;
    trans[2][0]= 0.0; trans[2][1]= 0.0; trans[2][2]= 1.0;
    
    // rotation
    
    double rotangle= vnl_math::pi/2 - vcl_atan2( ellipse2.y - ellipse1.y , ellipse2.x - ellipse1.x );
    vnl_matrix_fixed<double, 3, 3> rot;
    // rot= [ cos( rotangle ), -sin( rotangle ); sin( rotangle ), cos( rotangle ) ];
    double cosAng= vcl_cos( rotangle ), sinAng= vcl_sin( rotangle );
    rot[0][0]= cosAng; rot[0][1]= -sinAng; rot[0][2]= 0.0;
    rot[1][0]= sinAng; rot[1][1]=  cosAng; rot[1][2]= 0.0;
    rot[2][0]= 0.0;    rot[2][1]=  0.0;    rot[2][2]= 1.0;
    // Conic=[a,b;b,c]; ConicRot= inv(rot)' * Conic * inv(rot);
    // a=ConicRot(1,1); b=ConicRot(1,2); c=ConicRot(2,2);
    // also note inv(rot) = rot'
    double a= ellipse1.a*cosAng*cosAng - 2*ellipse1.b*cosAng*sinAng + ellipse1.c*sinAng*sinAng;
    double b= (ellipse1.a-ellipse1.c)*cosAng*sinAng + ellipse1.b*(cosAng*cosAng-sinAng*sinAng);
    double c= ellipse1.a*sinAng*sinAng + 2*ellipse1.b*cosAng*sinAng + ellipse1.c*cosAng*cosAng;
    
    vnl_matrix_fixed<double, 3, 3> Aff;
    double B= 0.0;
    double D= vcl_sqrt( c );
    double C= b/D;
    double A= vcl_sqrt(a-C*C);
    Aff[0][0]=   A; Aff[0][1]=   B; Aff[0][2]= 0.0;
    Aff[1][0]=   C; Aff[1][1]=   D; Aff[1][2]= 0.0;
    Aff[2][0]= 0.0; Aff[2][1]= 0.0; Aff[2][2]= 1.0;
    
    // calculate Haff
    vgl_h_matrix_2d<double> AffRot= Aff * rot;
    Haff= AffRot * trans;
    
    // transform ellipse2
    
    ellipse2transf= ellipse2;
    ellipse2transf.x -= ellipse1.x;
    ellipse2transf.y -= ellipse1.y;
    ellipse2transf.transformAffine( AffRot );
    
}
