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

#ifndef _EVALUATOR_H_
#define _EVALUATOR_H_

#include "par_queue.h"
#include <vector>
#include <set>
#include <stdint.h>
#include <string>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/set.hpp>

#include "retriever.h"
#include "query.h"
#include "document_map.h"


class evaluator {
    
    public:
        
        evaluator() : nQueries(0) {};
        
        evaluator( const char fileName[], bool aIgnoreQuery= false );
        
        evaluator( const char fileName[], documentMap &docMap, bool aIgnoreQuery= false );
        
        static void
            convertHolidays( const char fileName[], const char convertFrom[], documentMap &docMap );
        
        void
            saveToFile( const char fileName[] );
        
        void
            loadFromFile( const char fileName[] );
        
        typedef std::pair<double,double> APresultType;
        
        double
            computeMAP( retriever const &retriever_obj, std::vector<double> &APs, bool verbose= false, bool semiVerbose= false, parQueue<APresultType> *parQueue_obj= NULL ) const;
        
        double
            computeAP( uint32_t queryID, retriever const &retriever_obj, std::vector<double> &precision, std::vector<double> &recall, double &time ) const;
        
        
        uint32_t nQueries;
        std::vector<std::string> queryName;
        std::vector<query> queries;
        std::vector< std::set<uint32_t> > pos, ign;
        bool ignoreQuery;
    
    private:
        
        friend class boost::serialization::access;
        template<class A>
        void serialize(A & archive, const unsigned int version){ archive & nQueries & queryName & queries & pos & ign; }
        
};

#endif
