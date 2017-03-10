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

#ifndef _RELJA_SOLVE_QUARTIC_METHODS_H
#define _RELJA_SOLVE_QUARTIC_METHODS_H

#include <vcl_iostream.h> //temp
#include <vcl_complex.h>

#include <stdint.h>

class solve_quartic_methods {
    
    public:
        
        // from Wikipedia, cites Lodovico Ferrari's solution from 1540
        // solves ax^4+bx^3+cx^2+dx+e=0
        static void
            solve_quartic( double a, double b, double c, double d, double e,
                           vcl_complex<double> xs[], uint8_t &numsol );
                           
        static void
            solve_cubic( double a, double b, double c, double d,
                         vcl_complex<double> xs[], uint8_t &numsol );
        
        static void
            solve_quartic_real_unique( double a, double b, double c, double d, double e,
                                double xs[], uint8_t &numsol );
    
};


#endif //_RELJA_SOLVE_QUARTIC_METHODS_H
