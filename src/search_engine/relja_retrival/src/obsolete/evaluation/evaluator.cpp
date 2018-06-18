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

#include "evaluator.h"
#include "util.h"

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <map>

#include "timing.h"



evaluator::evaluator( const char fileName[], bool aIgnoreQuery ) : ignoreQuery(aIgnoreQuery) {
    loadFromFile( fileName );
}



evaluator::evaluator( const char fileName[], documentMap &docMap, bool aIgnoreQuery ) : ignoreQuery(aIgnoreQuery) {
    
    /*
    The format of the file is the following:
    1st line -- groundtruth id
    2nd line -- 'QUERY_ID QUERY_X1 QUERY_Y1 QUERY_X2 QUERY_Y2'
    3rd line -- [QUERY_PVE...] (positive examples)
    4th line -- [QUERY_AMB...] (ambiguous examples - don't care)
    Lines starting with # are treated as comments and ignored.
    */
    
    std::ifstream ifs(fileName);
    std::string line;
    short unsigned int state= 0;
    nQueries= 0;
    
    while (!ifs.eof()){
        getline(ifs,line);
        if (line[0]=='#') continue;
        switch (state){
            case 0: {
                // query name
                queryName.push_back(line);
                state= 1;
                break;
            }
            case 1: {
                // query image/region info
                std::istringstream lineStream(line);
                std::string hash;
                double xl, xu, yl, yu;
                lineStream >> hash >> xl >> yl >> xu >> yu;
                queries.push_back( query(docMap.h2i(hash),true,"",xl,xu,yl,yu) );
                state= 2;
                break;
            }
            case 2: {
                // positives
                std::istringstream lineStream(line);
                std::string hash;
                pos.push_back( std::set<uint32_t>() );
                while (!lineStream.eof()){
                    lineStream >> hash;
                    pos[nQueries].insert( docMap.h2i(hash) );
                }
                state= 3;
                break;
            }
            case 3: {
                // ignores
                std::istringstream lineStream(line);
                std::string hash;
                ign.push_back( std::set<uint32_t>() );
                while (!lineStream.eof()){
                    lineStream >> hash;
                    ign[nQueries].insert( docMap.h2i(hash) );
                }
                state= 0;
//                std::cout<< queryName[nQueries] <<": "<< queries[nQueries].docID <<" "<< pos[nQueries].size() << " " << ign[nQueries].size() << "\n";
                ++nQueries;
                break;
            }
        }
    }
    
    if (state!=0){
        throw std::runtime_error("Invalid groundtruth");
    }
    
}



class computeAPworker : public queueWorker<evaluator::APresultType> {
    public:
        computeAPworker( evaluator const *aEvaluatorObj, retriever const &aRetriever_obj ) : evaluatorObj(aEvaluatorObj), retriever_obj(&aRetriever_obj)
            {}
        void operator() ( uint32_t queryID, evaluator::APresultType &result ) const {
            std::vector<double> precision, recall;
            double queryTime;
            double AP= evaluatorObj->computeAP( queryID, *retriever_obj, precision, recall, queryTime );
            result= std::make_pair(AP,queryTime);
        }
    private:
        evaluator const *evaluatorObj;
        retriever const *retriever_obj;
};


class computeAPmanager : public queueManager<evaluator::APresultType> {
    public:
        computeAPmanager( std::vector<double> &aAPs, uint32_t aNQueries, std::vector<std::string> const &aQueryName, bool aVerbose, bool aSemiVerbose ) : APs(&aAPs), nQueries(aNQueries), queryName(&aQueryName), verbose(aVerbose), semiVerbose(aSemiVerbose), mAP(0.0), time(0.0), cumAP(0.0), times(aNQueries,-1.0), nextToPrint(0) {
            APs->clear();
            APs->resize(nQueries,0);
        }
        void operator()( uint32_t queryID, evaluator::APresultType &result ){
            double AP= result.first, queryTime= result.second;
            mAP+= AP;
            time+= queryTime;
            APs->at(queryID)= AP;
            times[queryID]= queryTime;
            if ( (verbose || semiVerbose) && nextToPrint==queryID){
                for (; nextToPrint<nQueries && times[nextToPrint]>-0.5; ++nextToPrint){
                    cumAP+= APs->at(nextToPrint);
                    if (verbose || (semiVerbose && nextToPrint%5==0))
                        printf("%.3d %s %.10f %.2f ms %.4f %.4f\n", nextToPrint, queryName->at(nextToPrint).c_str(), APs->at(nextToPrint), times[nextToPrint], cumAP/(nextToPrint+1), cumAP/nQueries );
                }
            }
        }
        std::vector<double> *APs;
        uint32_t nQueries;
        std::vector<std::string> const *queryName;
        bool verbose, semiVerbose;
        double mAP, time, cumAP;
        std::vector<double> times;
        uint32_t nextToPrint;
};



double
evaluator::computeMAP( retriever const &retriever_obj, std::vector<double> &APs, bool verbose, bool semiVerbose, parQueue<APresultType> *parQueue_obj ) const {
    
    bool deleteQueue= (parQueue_obj==NULL);
    if (deleteQueue)
        parQueue_obj= new parQueue<APresultType>(true,0);

    uint32_t const rank= parQueue_obj->getRank();
    
    verbose= verbose && (rank==0);
    semiVerbose= semiVerbose && !verbose && (rank==0);
    
    std::vector<double> precision, recall;
    
    if ( verbose || semiVerbose )
        printf("i query AP time mAP_proj mAP_sofar\n\n");
        
    computeAPworker computeAPworker_obj( this, retriever_obj );
    computeAPmanager *computeAPmanager_obj= (rank==0) ?
        new computeAPmanager( APs, nQueries, queryName, verbose, semiVerbose ) :
        NULL;
    parQueue_obj->start( nQueries, computeAPworker_obj, computeAPmanager_obj );
    
    double mAP= computeAPmanager_obj->mAP / nQueries;
    double time= computeAPmanager_obj->time;
    
    if (rank==0) delete computeAPmanager_obj;
    
    if ( verbose || semiVerbose )
        printf("\n\tmAP= %.10f, time= %.4f s, avgTime= %.4f ms\n\n", mAP, time/1000, time/nQueries);
    
    if (deleteQueue)
        delete parQueue_obj;
    
    return mAP;
    
}



double
evaluator::computeAP( uint32_t queryID, retriever const &retriever_obj, std::vector<double> &precision, std::vector<double> &recall, double &time ) const {
    
    uint32_t numPos= pos[queryID].size();
    
    precision.clear();
    precision.reserve( numPos );
    recall.clear();
    recall.reserve( numPos );
    
    uint32_t docID;
    
    uint32_t posSoFar= 0, nonIgnSoFar= 0;
    double AP= 0, currRec= 0, prevRec= 0, currPrec= 0;
    
    // query
    std::vector<indScorePair> queryRes;
    time= timing::tic();
    retriever_obj.queryExecute( queries[queryID], queryRes );
    time= timing::toc( time );
    
    std::set<uint32_t> prevDocs;
    
    for (std::vector< std::pair<uint32_t,double> >::iterator it= queryRes.begin(); it!=queryRes.end(); ++it){
        
        docID= it->first;
        
        if ( prevDocs.count( docID ) ){
            // already encountered so ignore (so that for example returning a positive 100 times doesn't boost results)
            continue;
        } else {
            // add to list of encountered
            prevDocs.insert( docID );
        }
        
        if ( (ignoreQuery && docID==queries[queryID].docID) ||
             ign[queryID].count( docID ) )
            continue;
        
        ++nonIgnSoFar;
        
        if ( pos[queryID].count( docID ) ) {
            
            ++posSoFar;
            currPrec= static_cast<double>(posSoFar)/nonIgnSoFar;
            currRec= static_cast<double>(posSoFar)/numPos;
            precision.push_back( currPrec );
            recall.push_back( currRec );
            
            AP+= (currRec-prevRec)*currPrec;
            prevRec= currRec;
            
        }
        
    }
    
    return AP;
    
}



void
evaluator::saveToFile( const char fileName[] ){
    
    std::ofstream ofs(fileName, std::ios::binary);
    boost::archive::binary_oarchive oa(ofs);
    oa << (*this);
    
}



void
evaluator::loadFromFile( const char fileName[] ){
    
    std::ifstream ifs(fileName, std::ios::binary);
    boost::archive::binary_iarchive ia(ifs);
    ia >> (*this);
    
}



void
evaluator::convertHolidays( const char fileName[], const char convertFrom[], documentMap &docMap ){
    
    throw std::runtime_error("Refer to evaluator_v2, shouldn't do ifs.eof()");
    evaluator ev;
    ev.nQueries= 0;
    ev.ignoreQuery= true;
    ev.queryName.clear(); ev.queries.clear(); ev.pos.clear(); ev.ign.clear();
    
    std::map<std::string,uint32_t> nameToID;
    uint32_t whereSlash;
    std::string fn;
    for (uint32_t docID=0; docID<docMap.numDocs(); ++docID){
        fn= docMap.getFn(docID);
        whereSlash= fn.rfind('/');
        fn= fn.substr( whereSlash+1, fn.length()-(whereSlash+1)-4 );
        nameToID[ fn ]= docID;
    }
    
    std::set<uint32_t> positives;
    
    std::ifstream ifs(convertFrom);
    std::string line;
    
    uint32_t docID;
    
    while (!ifs.eof()){
        
        getline(ifs,line);
        docID= nameToID[ line.substr(0,line.length()-4) ];
        
        if (line[line.length()-4-1]=='0' && line[line.length()-4-2]=='0'){
            // new query
            if (ev.nQueries!=0){
                ev.pos.push_back( positives );
                positives.clear();
            }
            ++ev.nQueries;
            // ignore the query
            std::set<uint32_t> ignores;
            ignores.insert(docID);
            ev.ign.push_back( ignores );
            ev.queries.push_back( docID );
            ev.queryName.push_back( line.substr(0,line.length()-4) );
        } else {
            // positives
            positives.insert( docID );
        }
        
    }
    // last positives:
    ev.pos.push_back( positives );
    
    
    ev.saveToFile( fileName );
    
    /*
    std::cout<<ev.nQueries<<"\n\n";
    
    for (uint32_t queryID= 0; queryID<5; ++queryID){
        
        std::cout<<queryID<<" "<<ev.queryName[queryID]<<" "<<docMap.getFn(ev.queries[queryID].docID)<<"\n";
        
        for (std::set<uint32_t>::iterator itPos=ev.pos[queryID].begin(); itPos!=ev.pos[queryID].end(); ++itPos)
            std::cout<<docMap.getFn(*itPos)<<" ";
        std::cout<<"\n";
        
        for (std::set<uint32_t>::iterator itIgn=ev.ign[queryID].begin(); itIgn!=ev.ign[queryID].end(); ++itIgn)
            std::cout<<docMap.getFn(*itIgn)<<" ";
        std::cout<<"\n\n";
        
    }
    */
    
}
