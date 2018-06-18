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

#ifndef _PLATT_SIGMOIDFIT_H_
#define _PLATT_SIGMOIDFIT_H_


#include <vector>


/*
 * based on
 * Platt "Probabilistic outputs for support vector machines and coparison to regularized likelihood methods"
 * and its improvement (pseudo code of which is implemented here):
 * "Lin, Lin, Weng "A note on Platt's probabilistic outputs for support vector machines"
 */

// deci= SVM decision values
// label= booleans is example labeled +1
void
platt_sigmoidfit( std::vector<double> const &deci, std::vector<bool> const &label, double &A, double &B);

#endif
