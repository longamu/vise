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

#include "platt_sigmoidfit.h"



#include <stdint.h>
#include <math.h>



void
platt_sigmoidfit( std::vector<double> const &deci, std::vector<bool> const &label, double &A, double &B) {
    
    uint32_t prior0= 0, prior1= 0;
    
    uint16_t maxiter= 100; // Maximum number of iterations
    double minstep= 1e-10; // Minimum step taken in line search
    double sigma= 1e-12; // Set to any value > 0
    
    // Construct initial values: target support in array t, initial function value in fval
    double hiTarget= (prior1+1.0)/(prior1+2.0);
    double loTarget= 1/(prior0+2.0);
    uint32_t len= label.size(); // Total number of data
    
    std::vector<double> t( len );
    
    for (uint32_t i= 0; i<len; ++i){
        prior0+= !label[i];
        prior1+= label[i];
        if (label[i] > 0)
            t[i]= hiTarget;
        else
            t[i]= loTarget;
    }
    
    A= 0.0;
    B= log((prior0+1.0)/(prior1+1.0));
    double fval= 0.0;
    
    for (uint32_t i= 0; i<len; ++i){
        double fApB= deci[i]*A+B;
        if (fApB >= 0)
            fval= fval + t[i]*fApB+log(1+exp(-fApB));
        else
            fval= fval + (t[i]-1)*fApB+log(1+exp(fApB));
    }
    
    for (uint16_t it= 0; it<maxiter; ++it){
        // Update Gradient and Hessian (use H’ = H + sigma I)
        double h11= sigma;
        double h22= sigma;
        double h21= 0.0;
        double g1= 0.0, g2=0.0;
        double p, q;
        
        for (uint32_t i= 0; i<len; ++i){
            double fApB= deci[i]*A+B;
            if (fApB >= 0) {
                p= exp(-fApB)/(1.0+exp(-fApB));
                q= 1.0/(1.0+exp(-fApB));
            } else {
                p= 1.0/(1.0+exp(fApB));
                q= exp(fApB)/(1.0+exp(fApB));
            }
            double d2= p*q;
            h11+= deci[i]*deci[i]*d2;
            h22+= d2;
            h21+= deci[i]*d2;
            double d1= t[i]-p;
            g1+= deci[i]*d1;
            g2+= d1;
        }
        
        if (fabs(g1)<1e-5 && fabs(g2)<1e-5) // Stopping criteria
            break;
        
        // Compute modified Newton directions
        double det= h11*h22-h21*h21;
        double dA= -(h22*g1-h21*g2)/det;
        double dB= -(-h21*g1+h11*g2)/det;
        double gd= g1*dA+g2*dB;
        double stepsize= 1;
        
        while (stepsize >= minstep) { // Line search
            double newA= A+stepsize*dA;
            double newB= B+stepsize*dB;
            double newf= 0.0;
            for (uint32_t i= 0; i<len; ++i){
                double fApB= deci[i]*newA+newB;
                if (fApB >= 0)
                    newf= newf + t[i]*fApB+log(1+exp(-fApB));
                else
                    newf= newf + (t[i]-1)*fApB+log(1+exp(fApB));
            }
            if (newf<fval+0.0001*stepsize*gd) {
                A= newA;
                B= newB;
                fval= newf;
                break; // Sufficient decrease satisfied
            } else
                stepsize= stepsize / 2.0;
        }
        
        if (stepsize < minstep){
            // Line search fails
            break;
        }
    }
    
    
    /// if (it >= maxiter)
        // Reaching maximum iterations
    
    
}
