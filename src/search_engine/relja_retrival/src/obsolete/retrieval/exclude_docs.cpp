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

#include "exclude_docs.h"

#include "query.h"


excludeDocs::excludeDocs( bowRetriever const &aRetriever_obj, evaluator const &evaluator_obj )
        : bowRetriever(aRetriever_obj.forwardIndex_obj,
                       aRetriever_obj.featGetter_obj,
                       aRetriever_obj.nn_obj,
                       aRetriever_obj.SA_obj),
          retriever_obj(&aRetriever_obj){;
    
    computeExcludes(evaluator_obj, excludes);
    
}



void
excludeDocs::computeExcludes( evaluator const &evaluator_obj, std::set<uint32_t> &aExcludes ){
    
    aExcludes.clear();
    
    for (std::vector<query>::const_iterator itQ= evaluator_obj.queries.begin(); itQ!=evaluator_obj.queries.end(); ++itQ){
        aExcludes.insert( itQ->docID );
    }
    
}



void
excludeDocs::queryExecute( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    
    if (toReturn!=0)
        toReturn+= excludes.size();
    retriever_obj->queryExecute( represent_obj, queryRes, toReturn );
    
    if (queryRes.size()==0) return;
    for (std::vector<indScorePair>::iterator itR= queryRes.end()-1; ; --itR ){
        if ( excludes.count(itR->first) )
            itR= queryRes.erase( itR );
        if (itR==queryRes.begin()) break;
    }
    
}

