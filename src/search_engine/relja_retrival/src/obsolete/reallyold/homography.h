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

#ifndef _RELJA_HOMOGRAPHY_H_
#define _RELJA_HOMOGRAPHY_H_

#include "ellipse.h"

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vgl/algo/vgl_h_matrix_2d.h>

#include <boost/serialization/split_member.hpp>
#include <boost/serialization/utility.hpp>

class homography {
    
    private:
        
        // C=[at, 0; bt, ct], such that C^T C = [a, b; b; c]
        inline static void
            cholesky( double a, double b, double c, double &at, double &bt, double &ct );
        // inv( [a, b; 0, c] )
        inline static void
            lowerTriInv( double a, double b, double c, double &at, double &bt, double &ct );
    
    public:
        
        //vgl_h_matrix_2d<double> *H;
        vgl_h_matrix_2d<double> H;
        
        // [0,0,0,0,0,0,0,0,1]
        homography();
        
        homography( float h[] );
        
        ~homography();
        
        // affine homography from matching ellipses using the gravity vector
        //homography( vgl_conic ellipse1, vgl_conic ellipse2 );
        homography( ellipse &ellipse1, ellipse &ellipse2 );
        
        double getDetAffine();
        
//         double getAffineEigRatio();
        double getSimEig();
        
            
        void exportToFloatArray( float h[] ) const {
            vnl_matrix_fixed <double, 3,3> hmat= H.get_matrix();
            h[0]= hmat[0][0]; h[1]= hmat[0][1]; h[2]= hmat[0][2];
            h[3]= hmat[1][0]; h[4]= hmat[1][1]; h[5]= hmat[1][2];
            h[6]= hmat[2][0]; h[7]= hmat[2][1]; h[8]= hmat[2][2];
        }
        
        void set( float h[] );
        
        inline void setIdentity(){
            H.set_identity();
        }
        
    private:
        
        friend class boost::serialization::access;
        template<class A>
        void save(A & archive, const unsigned int version) const {
            float h[9]; exportToFloatArray( h );
            archive & h[0] & h[1] & h[2]
                    & h[3] & h[4] & h[5]
                    & h[6] & h[7] & h[8];
        }
        template<class A>
        void load(A & archive, const unsigned int version) {
            float h[9];
            archive & h[0] & h[1] & h[2]
                    & h[3] & h[4] & h[5]
                    & h[6] & h[7] & h[8];
            set( h );
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()
    
};

#endif //_RELJA_HOMOGRAPHY_H_
