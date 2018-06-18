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

#ifndef _ELLIPSE_H_
#define _ELLIPSE_H_

#include <vcl_vector.h>

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_inverse.h>
#include <vgl/algo/vgl_h_matrix_2d.h>
#include <vgl/vgl_homg_point_2d.h>
#include <vcl_cmath.h>

#include <vcl_iostream.h> //temp

class ellipse {
    
    public:
        
        double x, y, a, b, c;
        
        ellipse( double aX,  double aY, double aA, double aB, double aC );
        
        ellipse() : x(0),y(0),a(0),b(0),c(0) {}
        
        inline void
            getCentre( vgl_homg_point_2d<double> &centre );
        
        static void
            getCentres( vcl_vector< ellipse > &ellipses,
                        vcl_vector< vgl_homg_point_2d<double> > &centres );
        
        // assuming C=[a b 0; b c 0; 0 0 -1] => Cdual= [ inv([a b; b c]) 0; 0 0 -1 ]
        inline void
            getDual( double &adual, double &bdual, double &cdual );
        
        // assumes Haff=[p q 0; r s 0; 0 0 t]
        inline void
            transformAffine( vgl_h_matrix_2d<double> &Haff );
        
        // angle is the angle of the two tangents, i.e. tangents are (homogeneous) lines of the form [-sin(angle); cos(angle); d]
        inline double
            getDistBetweenTangents( double cosAngle, double sinAngle );
        
        // a*c-b^2
        inline double
            getPropAreaSq();
        
};


inline void
ellipse::getCentre( vgl_homg_point_2d<double> &centre ){
    centre.set(x,y,1.0);
}


inline void
ellipse::getDual( double &adual, double &bdual, double &cdual ){
    double det= a*c-b*b;
    adual=  c/det;
    bdual= -b/det;
    cdual=  a/det;
}


inline void
ellipse::transformAffine( vgl_h_matrix_2d<double> &Haff ){
    
    // transform centre
    vgl_homg_point_2d<double> centre(x,y,1.0), centret;
    centret= Haff*centre;
    
    // transform a,b,c
    vnl_matrix_fixed<double,3,3> C0, C0t, Tinv;
    
    C0[0][0]=   a; C0[0][1]=   b; C0[0][2]=  0.0;
    C0[1][0]=   b; C0[1][1]=   c; C0[1][2]=  0.0;
    C0[2][0]= 0.0; C0[2][1]= 0.0; C0[2][2]= -1.0;
    
    vnl_matrix_fixed<double,3,3> Hinv= vnl_inverse( Haff.get_matrix() );
    Hinv= Hinv/Hinv[2][2];
    // translate Hinv (same as translating C but more efficient)
    Hinv[0][2]-= x;
    Hinv[1][2]-= y;
    C0t= Hinv.transpose()*C0*Hinv;
    
    // now translate C0t to Ct by xt,yt, but the following is not affected
    a= C0t[0][0];
    b= C0t[0][1];
    c= C0t[1][1];
    x= centret.x()/centret.w();
    y= centret.y()/centret.w();
    
}

inline double
ellipse::getDistBetweenTangents( double cosAngle, double sinAngle ){
    double det= a*c-b*b;
    double duala= c/det, dualb= -b/det, dualc= a/det;
    // dualC=[ duala dualb 0; dualb dualc 0; 0 0 -1 ]
    // l=[-sin(angle); cos(angle); d]
    // l'*dualC*l=0 =>
    // d= duala*duala*sinAngle - 2*dualb*sinAngle*cosAngle + dualc*dualc*cosAngle*cosAngle;
    // return 2*sqrt(d)
    return 2*vcl_sqrt( duala*duala*sinAngle - 2*dualb*sinAngle*cosAngle + dualc*dualc*cosAngle*cosAngle );
}

inline double
ellipse::getPropAreaSq(){
    return a*c-b*b;
}

#endif
