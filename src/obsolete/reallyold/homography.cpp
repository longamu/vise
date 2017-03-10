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

#include "homography.h"


#include <vnl/vnl_matrix.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vgl/algo/vgl_h_matrix_2d.h>


homography::homography( ){
    vnl_matrix_fixed <double, 3,3> h;
    h[0][0]= 0; h[0][1]= 0; h[0][2]= 0;
    h[1][0]= 0; h[1][1]= 0; h[1][2]= 0;
    h[2][0]= 0; h[2][1]= 0; h[2][2]= 1;
    
    //H= new vgl_h_matrix_2d <double> (h);
    H= vgl_h_matrix_2d<double>(h);
}

homography::homography( float aH[] ){
    set( aH );
}

void
homography::set( float aH[] ){
    vnl_matrix_fixed <double, 3,3> h;
    h[0][0]= aH[0]; h[0][1]= aH[1]; h[0][2]= aH[2];
    h[1][0]= aH[3]; h[1][1]= aH[4]; h[1][2]= aH[5];
    h[2][0]= aH[6]; h[2][1]= aH[7]; h[2][2]= aH[8];
    
    //H= new vgl_h_matrix_2d <double> (h);
    H= vgl_h_matrix_2d<double>(h);
}



homography::homography( ellipse &el1, ellipse &el2 ){

    double ma1, mb1, mc1, ma2, mb2, mc2, ma2i, mb2i, mc2i, a, b, c, tx, ty;

    // as in James's RANSAC:
    // A1= C1^T C1
    homography::cholesky( el1.a, el1.b, el1.c, ma1, mb1, mc1 );
    // A2= C2^T C2
    homography::cholesky( el2.a, el2.b, el2.c, ma2, mb2, mc2 );
    
    homography::lowerTriInv( ma2, mb2, mc2, ma2i, mb2i, mc2i );
    
    // H= C2^(-1) C1
    a = ma1*ma2i;
    c = mc1*mc2i;
    b = ma1*mb2i + mb1*mc2i;

    tx = el2.x - a*el1.x;
    ty = el2.y - b*el1.x - c*el1.y;

    vnl_matrix_fixed <double, 3,3> h;
    h[0][0]= a; h[0][1]= 0; h[0][2]= tx;
    h[1][0]= b; h[1][1]= c; h[1][2]= ty;
    h[2][0]= 0; h[2][1]= 0; h[2][2]=  1;
    
    //H= new vgl_h_matrix_2d <double> (h);
    H= vgl_h_matrix_2d <double> (h);
    
}

homography::~homography(){
    //delete H;
}


void
homography::cholesky( double a, double b, double c, double &at, double &bt, double &ct ){
    ct= sqrt(c);
    bt= b/ct;
    at= sqrt(a-bt*bt);
}

void
homography::lowerTriInv( double a, double b, double c, double &at, double &bt, double &ct ){
    double invdet = 1.0/(a*c);
    at=  c*invdet;
    bt= -b*invdet;
    ct=  a*invdet;
}

double
homography::getDetAffine(){
    //vnl_matrix_fixed<double,3,3> h=H->get_matrix();
    vnl_matrix_fixed<double,3,3> h=H.get_matrix();
    return (h[0][0] * h[1][1] - h[0][1] * h[1][0])/(h[2][2]*h[2][2]);
}

/*
double
homography::getAffineEigRatio(){
    vnl_matrix_fixed<double,3,3> h=H.get_matrix();
    double b= -(h[0][0]+h[1][1])/h[2][2];
    double c= getDetAffine();
    double lam1= (-b+sqrt(b*b-4*c))/2;
    double lam2= (-b-sqrt(b*b-4*c))/2;
    std::cout<<" "<<b*b-4*c<<" ";
    return lam1/lam2;
}
*/

double homography::getSimEig(){
    vnl_matrix_fixed<double,3,3> h=H.get_matrix();
    return pow( (h[0][0]+h[1][1])/h[2][2] ,2 ) - 4*getDetAffine();
}
