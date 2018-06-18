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

#ifndef _FIDX_WRAPPER_JP_DB5_H_
#define _FIDX_WRAPPER_JP_DB5_H_

#include <vector>
#include <stdint.h>

#include "forward_index.h"
#include "jp_db5.hpp"
#include "document_map.h"
#include "soft_assigner.h"
#include "desc_getter_from_file.h"


class fidxWrapperJpDb5 : public forwardIndex {
    
    public:
        
        fidxWrapperJpDb5( const char fileName[], documentMap &aDocMap, const char mode[]= "r" );
        
        virtual ~fidxWrapperJpDb5() { jp_db5_obj.close(); }
        
        void
            getWordsRegs( uint32_t docID, std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const;
        
        void
            setWordsRegs( uint32_t docID, std::vector<quantDesc> const &words, std::vector<ellipse> const &regions );
        
        inline uint32_t
            numDocs() const { return numDocs_; }
        
        friend class fidxWrapperJpDb5_FixK;
    
    protected:
        
        documentMap *docMap;
        
        jp_db5 jp_db5_obj;
        uint32_t numDocs_;
        
        
        virtual void
            compose_data( char *&p, uint32_t &sz, 
                          std::vector<quantDesc> const &words, std::vector<ellipse> const &regions )= 0;
        
        virtual void
            decompose_data( char *p, uint32_t sz, 
                            std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const = 0;
            
};




class fidxWrapperJpDb5_HardJP : public fidxWrapperJpDb5 {
    
    public:
        
        fidxWrapperJpDb5_HardJP( const char fileName[], documentMap &aDocMap, const char mode[]= "r" ) :
            fidxWrapperJpDb5( fileName, aDocMap, mode )
                {}
        
    protected:
        
        void
            decompose_data( char *p, uint32_t sz, 
                            std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const;
        
        void
            compose_data( char *&p, uint32_t &sz, 
                          std::vector<quantDesc> const &words, std::vector<ellipse> const &regions ) {
                std::cout<< "NOT IMPLEMENTED\n"; // see fidxWrapperJpDb5_FixK
            }
    
};



class fidxWrapperJpDb5_FixK : public fidxWrapperJpDb5 {
    
    public:
        
        fidxWrapperJpDb5_FixK( const char fileName[], documentMap &aDocMap, uint aK= 1, const char mode[]= "r", bool aDoNormalize= true ) :
            fidxWrapperJpDb5( fileName, aDocMap, mode ), K(aK), hasWeights(true), doNormalize(aDoNormalize) {
                if (K==0){
                    K= 1;
                    hasWeights= false;
                }
            }
        
        static void
            computeWords( documentMap &aDocMap, const char wordFn[], descGetterFromFile *desc_obj2, const char featFn[], const char asgnFn[], uint KNN= 1, softAssigner *SA_obj= NULL );
        
        static void
            convertFromHardJP( fidxWrapperJpDb5_HardJP &fidxFrom, const char fileName[], uint K= 0 );
        
    protected:
        
        void
            compose_data( char *&p, uint32_t &sz, 
                          std::vector<quantDesc> const &words, std::vector<ellipse> const &regions );
        
        void
            decompose_data( char *p, uint32_t sz, 
                            std::vector<quantDesc> &words, std::vector<ellipse> &regions ) const;
        
        void
            decomposeAsgn( char *p, uint32_t sz, uint KNN, std::vector<quantDesc> &words, softAssigner *SA_obj= NULL, float *feats= NULL ) const;
        
        static void
            decomposeReg( char *p, uint32_t sz, std::vector<ellipse> &regions );
        
        //temp
        void
            softAsgn( quantDesc &ww );
        
        uint K;
        bool hasWeights, doNormalize;
    
};

#endif
