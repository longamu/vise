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

#include "solve_quartic_methods.h"

#include <vcl_iostream.h> //temp
#include <vcl_complex.h>

#include <stdint.h>

void
solve_quartic_methods::solve_quartic_real_unique( double a, double b, double c, double d, double e,
                                                  double xs[], uint8_t &numsol ){
    
    vcl_complex<double> complex_xs[4];
    uint8_t complex_numsol;
    solve_quartic_methods::solve_quartic( a, b, c, d, e, complex_xs, complex_numsol );
    
    uint8_t i, j;
    double sol;
    numsol=0;
    for (i=0; i<complex_numsol; i++){
        // real?
        if ( vcl_abs(complex_xs[i].imag())<1e-5 ){
            sol= complex_xs[i].real();
            // unique?
            for (j=0; j<numsol && xs[j]!=sol; j++);
            if (j>=numsol){
                xs[numsol]= sol;
                numsol++;
            }
        }
    }
    
}

// from Wikipedia, cites Lodovico Ferrari's solution from 1540
// solves ax^4+bx^3+cx^2+dx+e=0
void
solve_quartic_methods::solve_quartic( double a, double b, double c, double d, double e,
                                      vcl_complex<double> xs[], uint8_t &numsol ){
    
    numsol=0;
    
    if (a != 0){
        
        // solve quartic
        
        e= e/a; d= d/a; c= c/a; b= b/a; a=1;
        double alpha= -3*b*b/8 + c;
        double beta = b*b*b/8 - b*c/2 + d;
        double gamma= -3*b*b*b*b/256 + c*b*b/16 - b*d/4 + e;
        if (beta==0){
            /*
            xs =[ ...
                -b/4 + sqrt( (-alpha + sqrt( alpha^2 - 4*gamma ) ) / 2 ); ...
                -b/4 + sqrt( (-alpha - sqrt( alpha^2 - 4*gamma ) ) / 2 ); ...
                -b/4 - sqrt( (-alpha + sqrt( alpha^2 - 4*gamma ) ) / 2 ); ...
                -b/4 - sqrt( (-alpha - sqrt( alpha^2 - 4*gamma ) ) / 2 )  ...
                ];
                */
            vcl_complex<double> innerSqrt= vcl_sqrt( vcl_complex<double>(alpha*alpha - 4*gamma) );
            xs[0]= vcl_sqrt( 0.5*(-alpha + innerSqrt ) );
            xs[1]= vcl_sqrt( 0.5*(-alpha - innerSqrt ) );
            xs[2]= xs[0];
            xs[3]= xs[1];
            xs[0] = -b/4 + xs[0];
            xs[1] = -b/4 + xs[1];
            xs[2] = -b/4 - xs[2];
            xs[3] = -b/4 - xs[3];
            numsol= 4;
        } else {
            double P= -alpha*alpha/12 - gamma;
            double Q= -alpha*alpha*alpha/108 + alpha*gamma/3 - beta*beta/8;
            vcl_complex<double> R= -Q/2 + vcl_sqrt( vcl_complex<double>( Q*Q/4 + P*P*P/27) );
            vcl_complex<double> y;
            if (vcl_abs(R)==0){
                y= -5*alpha/6 - vcl_pow( Q, 1.0/3 );
            } else {
                vcl_complex<double> U= vcl_pow( R, 1.0/3 );
                y= -5*alpha/6 + U - P/(3.0*U);
            }
            vcl_complex<double> W= vcl_sqrt( alpha + 2.0*y );
            
            /*
            xs =[ ...
                -b/4 + ( + W + sqrt( -( 3*alpha +2*y + 2*beta/W ) ) )/2; ...
                -b/4 + ( + W - sqrt( -( 3*alpha +2*y + 2*beta/W ) ) )/2; ...
                -b/4 + ( - W + sqrt( -( 3*alpha +2*y - 2*beta/W ) ) )/2; ...
                -b/4 + ( - W - sqrt( -( 3*alpha +2*y - 2*beta/W ) ) )/2; ...
                ];
                */
            xs[0]= -b/4 + 0.5*( + W + vcl_sqrt( vcl_complex<double>( -( 3*alpha +2.0*y + 2*beta/W ) ) ) );
            xs[1]= -b/4 + 0.5*( + W - vcl_sqrt( vcl_complex<double>( -( 3*alpha +2.0*y + 2*beta/W ) ) ) );
            xs[2]= -b/4 + 0.5*( - W + vcl_sqrt( vcl_complex<double>( -( 3*alpha +2.0*y - 2*beta/W ) ) ) );
            xs[3]= -b/4 + 0.5*( - W - vcl_sqrt( vcl_complex<double>( -( 3*alpha +2.0*y - 2*beta/W ) ) ) );
            numsol=4;   
        }
        
    }
    else {
        solve_quartic_methods::solve_cubic( b, c, d, e, xs, numsol );
    }
    
}


// solve cubic: ax^3+bx^2+cx+d=0
void
solve_quartic_methods::solve_cubic( double a, double b, double c, double d,
                                    vcl_complex<double> xs[], uint8_t &numsol ){
    
    if ( a!=0 ){
        
        // solve cubic
        
        double nrm= a;
        a= b/nrm; b= c/nrm; c= d/nrm;
        // solve cubic: x^3+ax^2+bx+c=0, solutions from wiki
        double m= 2*a*a*a - 9*a*b + 27*c;
        double k= a*a - 3*b;
        vcl_complex<double> n= m*m - 4*k*k*k;
        vcl_complex<double> term1_= vcl_pow( 0.5*(m+vcl_sqrt(n)), 1.0/3 );
        vcl_complex<double> term2_= vcl_pow( 0.5*(m-vcl_sqrt(n)), 1.0/3);
        
        // need to chose term1*term2==k since there are 3 possible values for each term
        vcl_complex<double> rs[3], i= vcl_complex<double>(0.0,1.0);
        rs[0]= 1; rs[1]= 0.5*(1.0+i*vcl_sqrt(3.0)); rs[2]= 0.5*(1.0-i*vcl_sqrt(3.0));
        double minval=10000, val;
        vcl_complex<double> term1= term1_, term2=term2_;
        for(int root1=0; root1<3; root1++){
            for(int root2=0; root2<3; root2++){
                val= vcl_abs( rs[root1]*term1_ * rs[root2]*term2_ - k);
                if (val<minval) {
                    minval= val;
                    term1= rs[root1]*term1_;
                    term2= rs[root2]*term2_;                }
            }
        }
        
        vcl_complex<double> w1= 0.5*(-1.0+i*sqrt(3));
        vcl_complex<double> w2= 0.5*(-1.0-i*sqrt(3));
        /*
        xs= [ ...
            -1.0/3*( a +    term1 +    term2 ); ...
            -1.0/3*( a + w2*term1 + w1*term2 ); ...
            -1.0/3*( a + w1*term1 + w2*term2 ); ...
            ];
        */
        xs[0]= -1.0/3*( a +    term1 +    term2 );
        xs[1]= -1.0/3*( a + w2*term1 + w1*term2 );
        xs[2]= -1.0/3*( a + w1*term1 + w2*term2 );
        numsol=3;
        
    } else {
        
        // solve quadratic: bx^2+cx+d=0
        if (b!=0){
            c= d/b; b= c/b; a= 1;
            xs[0]= 0.5*(-b + vcl_sqrt( vcl_complex<double>( b*b - 4*c ) ));
            xs[1]= 0.5*(-b - vcl_sqrt( vcl_complex<double>( b*b - 4*c ) ));
            numsol=2;
        } else {
            // solve cx+d=0
            if (c!=0){
                xs[0]= -d/c;
                numsol=1;
            } else {
                if (d!=0){
                    numsol=0;
                } else {
                    xs[0]= 0; // all solutions are possible, return only 1?
                    numsol=1;
                }
            }
        }
        
    }
    
    
}
