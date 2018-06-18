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

#include "svm_util.h"

#include "macros.h"




void
svmUtil::addSVMData( svm_problem &problem, std::map<uint32_t,int> const &wordToDim, int indData, bool positive, std::map<uint32_t,double> const &BoW ){
    
    problem.y[indData]= positive?+1.0:-1.0;
    problem.x[indData]= new svm_node[ BoW.size()+1 ];
    int j= 0;
    for (std::map<uint32_t,double>::const_iterator itW= BoW.begin();
         itW!=BoW.end();
         ++itW, ++j){
        problem.x[indData][j].index= wordToDim.at(itW->first);
        if (j>0)
            ASSERT( problem.x[indData][j].index > problem.x[indData][j-1].index);
        problem.x[indData][j].value= itW->second;
    }
    problem.x[indData][j].index= -1; //end
    
}
