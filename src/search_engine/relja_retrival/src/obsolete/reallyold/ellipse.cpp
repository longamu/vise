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

#include "ellipse.h"

#include <vcl_vector.h>
#include <vgl/vgl_homg_point_2d.h>


ellipse::ellipse( double aX,  double aY, double aA, double aB, double aC ){
    x=aX; y=aY;
    a=aA; b=aB; c=aC;
}


void
ellipse::getCentres( vcl_vector< ellipse > &ellipses,
                     vcl_vector< vgl_homg_point_2d<double> > &centres ){
    
    centres.clear();
    centres.reserve( ellipses.size() );
    
    vgl_homg_point_2d<double> centre;
    
    for ( vcl_vector<ellipse>::iterator it=ellipses.begin(); it!=ellipses.end(); ++it ){
        it->getCentre( centre );
        centres.push_back( centre );
    }
        
    
}