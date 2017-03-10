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

#include "multi_query.h"



void
multiQueryIndpt::query( retsType &represent_objs, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    
    queryRes.clear();
    
    uint32_t numDocs= retriever_obj->numDocs();
    
    std::vector<indScorePair> thisRes;
    thisRes.reserve( toReturn );
    
    std::vector<double> scores(numDocs);
    
    mqIndpt_worker worker( *retriever_obj, represent_objs, workerReturnOnlyTop() ? toReturn : 0 );
    queueManager<Result> *manager= getManager( scores );
    
    threadQueue<Result>::start( represent_objs.size(), worker, *manager, represent_objs.size() );
    
    delete manager;
    
    retriever::sortResults( scores, queryRes, toReturn );
    
}



void
multiQueryIndpt::mqIndpt_worker::operator() ( uint32_t jobID, Result &result ) const {
    
    // copy the represent object - querying might change it and with multithreading this can mess things up; change this if retriever.h ever specifies represent_obj to be const
    represent thisRep= *(represent_objs->at(jobID));
    
    // manager should delete this
    result= new std::vector<indScorePair>;
    result->reserve( toReturn );
    
    // do the query
    retriever_obj->queryExecute( thisRep, *result, toReturn );
    
}



void
multiQueryMax::mqMax_manager::operator() ( uint32_t jobID, Result &result ) {
    
    // TODO keep a sorted list of results and track the top toReturn, instead of tracking everything and then re-sorting
    for (std::vector<indScorePair>::const_iterator itR= result->begin(); itR!=result->end(); ++itR){
        if (first || itR->second > scores->at( itR->first ) )
            scores->at( itR->first )= itR->second;
    }
    
    first= false;
    
    delete result;
    
}



void
multiQuerySum::mqSum_manager::operator() ( uint32_t jobID, Result &result ) {
    
    // TODO keep a sorted list of results and track the top toReturn, instead of tracking everything and then re-sorting
    for (std::vector<indScorePair>::const_iterator itR= result->begin(); itR!=result->end(); ++itR){
        scores->at( itR->first )+= itR->second;
    }
    
    delete result;
    
}



void
multiQueryJointAvg::query( retsType &represent_objs, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    
    represent avg;
    for (retsType::iterator itRep= represent_objs.begin(); itRep!=represent_objs.end(); ++itRep){
        
        for (std::map<uint32_t,double>::iterator itBe= (*itRep)->BoW.begin(); itBe!=(*itRep)->BoW.end(); ++itBe)
            avg.BoW[itBe->first] += itBe->second;
        
    }
    
    tfidf_obj->queryExecute( avg, queryRes, toReturn );
    
}



void
multiQueryPostSpatial::query( retsType &represent_objs, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    
    firstMultiQuery_obj->query( represent_objs, queryRes, toReturn );
    
    spatWorker worker( represent_objs, *spatVerif_obj, queryRes );
    spatManager manager( queryRes );
    
    threadQueue<double>::start(
        std::min( static_cast<uint32_t>( queryRes.size() ), spatVerif_obj->spatialDepth() ),
        worker, manager, represent_objs.size()
    );
    
    retriever::sortResults( queryRes, spatVerif_obj->spatialDepth(), toReturn );
    
}
