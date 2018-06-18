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

#ifndef _EXCLUDE_DOCS_H_
#define _EXCLUDE_DOCS_H_

#include <vector>
#include <set>
#include <stdint.h>

#include "bow_retriever.h"
#include "query.h"
#include "represent.h"
#include "evaluator.h"
#include "forward_index.h"

class excludeDocs : public bowRetriever {
    
    public:
        
        excludeDocs( bowRetriever const &aRetriever_obj, evaluator const &evaluator_obj );
        
        void
            queryExecute( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
        static void
            computeExcludes( evaluator const &evaluator_obj, std::set<uint32_t> &aExcludes );
        
        std::set<uint32_t> excludes;
    
    private:
        
        bowRetriever const *retriever_obj;
    
};

#endif
