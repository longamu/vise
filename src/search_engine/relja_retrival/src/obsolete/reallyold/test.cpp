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

#include "ransac.h"
#include "fit_affine.h"
#include "fit_fund_affine4.h"
#include "fit_fund_affine2.h"
#include "ellipse.h"

#include <vcl_vector.h>
#include <vcl_utility.h>
#include <vcl_cstdio.h>
#include <vcl_string.h>

#include <stdint.h>

int main( int argc, char *argv[] ){
    
    
    vcl_vector< ellipse > ellipses1, ellipses2;
    
    double x,y,a,b,c,theta;
    vcl_FILE *f= vcl_fopen("data/inliers_3d_02.m","r");
    while (!vcl_feof(f)){
        ssize_t tmpout_= 0; // Relja: to remove warning that we're ignoring outputs
        tmpout_= vcl_fscanf(f, "%lf, %lf, %lf, %lf, %lf, %lf, ...\n", &x, &y, &a, &b, &c, &theta );
        //vcl_printf("%lf ",x);
        ellipses1.push_back( ellipse(x, y, a, b, c) );
        tmpout_= vcl_fscanf(f, "%lf, %lf, %lf, %lf, %lf, %lf;\n", &x, &y, &a, &b, &c, &theta );
        ellipses2.push_back( ellipse(x, y, a, b, c) );
    }
    vcl_fclose(f);
    
    vcl_vector< vcl_pair<uint32_t, uint32_t> > putativeMatches;
    putativeMatches.reserve( ellipses1.size() );
    for (uint32_t i=0; i<ellipses1.size(); i++)
        putativeMatches.push_back( vcl_make_pair(i,i) );
    
    if ( argc<2 || vcl_strcmp(argv[1],"affine")==0 ){
        
        vcl_printf("affine\n\n");
    
        fit_affine affineObj( ellipses1, ellipses2,
                              putativeMatches,
                              40.0, 1.0/1.75, 1.75,
                              true);
                          
        ransac::doRansac( putativeMatches, affineObj, 1, 1, true );
        
    } else
    if ( vcl_strcmp(argv[1],"fund_affine4")==0 ){
        
        vcl_printf("fund_affine4\n\n");
    
        fit_fund_affine4 fitterObj(
            ellipses1, ellipses2,
            putativeMatches,
            40.0, 4.0,
            true);
                            
        ransac::doRansac( putativeMatches, fitterObj, 1, 1, true );
        
    }
    else {
        
        vcl_printf("fund_affine2\n\n");
        
        fit_fund_affine2 fitterObj(
            ellipses1, ellipses2,
            putativeMatches,
            40.0, 4.0,
            true);
            
         ransac::doRansac( putativeMatches, fitterObj, 1, 1, true );
    }
        
    
    return 0;
}