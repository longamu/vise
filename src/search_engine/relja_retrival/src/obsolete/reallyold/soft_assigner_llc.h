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

#ifndef _SOFT_ASSIGNER_LLC_
#define _SOFT_ASSIGNER_LLC_

#include "quant_desc.h"
#include "soft_assigner.h"
#include "clst_centres.h"



class SA_LLC : public softAssigner {
    
    public:
        
        SA_LLC( clstCentres *aClstC, float aSigmaSq= 6250, double aLambda= 0.1 ) : sigmaSq(aSigmaSq), lambda(aLambda), clstC(aClstC)
            {}
        
        bool
            needsFeat() const { return true; }
        
        void
            getWeights( quantDesc &ww, float *feat );
    
    private:
        
        const float sigmaSq;
        const double lambda;
        clstCentres *clstC;
    
};

#endif
