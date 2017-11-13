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

#ifndef _NN_EVALUATOR_
#define _NN_EVALUATOR_

#include <cstddef> // for NULL
#include <stdint.h>
#include <string>
#include <vector>

#include "nn_searcher.h"



class nnEvaluator {
    
    public:
        
        nnEvaluator(std::string queryFn= "~/Relja/Data/Jegou_ANN/ANN_SIFT1M/sift_query.fvecs");
        
        ~nnEvaluator();
        
        double
            computeAverageRec( nnSearcher const &nnSearcher_obj, uint32_t recallAt= 100, std::vector<double> *recs= NULL, bool verbose= false, bool semiVerbose= false ) const;
        
        double
            computeRecall( uint32_t queryID, nnSearcher const &nnSearcher_obj, uint32_t recallAt, double &time ) const;
        
        typedef std::pair<double,double> recResultType;
        
    private:
        
        uint32_t nQueries, numDims;
        uint32_t **gt;
        float **qVecs;
    
};

#endif
