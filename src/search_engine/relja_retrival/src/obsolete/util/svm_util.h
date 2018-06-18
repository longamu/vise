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

#ifndef _SVM_UTIL_H_
#define _SVM_UTIL_H_

#include <vector>
#include <map>
#include <stdint.h>

#include "svm.h"


class svmUtil {
    
    public:
        
        static void
            addSVMData( svm_problem &problem, std::map<uint32_t,int> const &wordToDim, int indData, bool positive, std::map<uint32_t,double> const &BoW );
};


#endif
