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

#include "query_expand_svm.h"

#include <limits.h>
#include <set>



void
queryExpandSVM::expandAndQuery(
        represent &represent_obj,
        std::vector<indScorePair> &queryRes,
        std::map<uint32_t,homography> &Hs,
        bool isInternal,
        uint32_t numVerif,
        std::set<uint32_t> &inPrepend,
        uint32_t toReturn ) const {
    
    std::map<uint32_t,double> w;
    double b;
    
    if ( expandSVM( iidx, forwardIndexDB_obj, tfidf_obj->idf, represent_obj, queryRes, Hs, isInternal, numVerif, w, b ) ) {
        
        //----- Re-query with new weights
        
        std::vector<double> scores;
        weighter::queryExecute( w, tfidf_obj, scores, b );
        util::normTo0_1( scores );
        retriever::sortResults( scores, queryRes, toReturn );
        spatialVerif_obj->spatialQuery( represent_obj, queryRes, Hs, &inPrepend, toReturn, false );
        
    }
    
}



bool
queryExpandSVM::expandSVM(
        invertedIndex const *iidx,
        forwardIndex const *forwardIndexDB_obj,
        std::vector<double> &idf,
        represent &represent_obj,
        std::vector<indScorePair> &queryRes,
        std::map<uint32_t,homography> &Hs,
        bool isInternal,
        uint32_t numVerif,
        std::map<uint32_t,double> &w,
        double &b ){
    
    uint32_t maxNumNeg= 200;
    double maxNegScore= 0.2;
    uint32_t numPos= 1 + numVerif-isInternal; // +1 for query
    
    std::vector< BoWtype > posData;
    std::set< uint32_t > wordsUsed;
    posData.reserve( numPos );
    
    uint32_t docIDexpand;
    uint32_t wordsStart;
    
    //----- positive training data
    
    // add query document
    
    posData.resize( 1 );
    represent::computeBoW( represent_obj.words, posData.back() );
    // add wordIDs into the list of words used in training
    for (BoWtype::iterator itW= posData.back().begin();
         itW!=posData.back().end();
         ++itW){
        wordsUsed.insert( itW->first );
    }
    
    // expand documents
    
    for (uint32_t iRes= isInternal; iRes < numVerif; ++iRes){
        
        docIDexpand= queryRes[iRes].first;
        
        wordsStart= represent_obj.words.size();
        represent_obj.add( *forwardIndexDB_obj, docIDexpand, &Hs[docIDexpand], false );
        
        // words from this document that were used for expansion (e.g. fit into the query region)
        std::vector<quantDesc> wordsExpand( represent_obj.words.begin()+wordsStart, represent_obj.words.end() );
        
        // compute expand BoW
        posData.resize( posData.size()+1 );
        represent::computeBoW( wordsExpand, posData.back() );
        
        // add wordIDs into the list of words used in training
        for (BoWtype::iterator itW= posData.back().begin();
             itW!=posData.back().end();
             ++itW){
            wordsUsed.insert( itW->first );
        }
        
    }
    
    //----- usedWord - dimension in feature/weight vector
    if (wordsUsed.size() > INT_MAX){
        std::cerr<<"PROBLEM: more words used than svm training can handle (>maxint)\n";
    }
    std::map<uint32_t,int> wordToDim;
    std::vector<uint32_t> dimToWord;
    dimToWord.reserve( wordsUsed.size() );
    int numDim= 0;
    for (std::set<uint32_t>::iterator itW= wordsUsed.begin();
         itW!=wordsUsed.end();
         ++itW, ++numDim) {
        wordToDim[ *itW ]= numDim;
        dimToWord.push_back( *itW );
    }
    
    //----- negative training data
    
    std::map< uint32_t, BoWtype > negData;
    
    // can do binary search but probably not worth it
    uint32_t lastNeg;
    for( lastNeg= queryRes.size()-1; lastNeg > numVerif && queryRes[lastNeg].second < 1e-7; --lastNeg);
    
    {
    BoWtype BoWempty;
    for (uint32_t iRes= lastNeg;
         iRes > numVerif &&
         iRes + maxNumNeg > lastNeg &&
         queryRes[iRes].second < maxNegScore;
         --iRes )
        negData[ queryRes[iRes].first ]= BoWempty;
    }
    
    {
    // get training data
    std::vector< docFreqPair > docFreq;
    std::vector< docFreqPair >::iterator itDF;
    uint32_t docIDneg;
    
    // go through used words and iidx to get counts for each negative document
    for (std::set<uint32_t>::iterator itW= wordsUsed.begin();
         itW!=wordsUsed.end();
         ++itW) {
        
        iidx->getDocFreq( *itW, docFreq );
        for (itDF= docFreq.begin(); itDF!=docFreq.end(); ++itDF){
            docIDneg= itDF->first;
            if ( negData.count( docIDneg ) ){
                negData[docIDneg][ *itW ]+= itDF->second;
            }
        }
        
    }
    
    // append negatives to all training data
    }
    
    uint32_t numNeg= negData.size();
    if (numNeg==0){
        std::cerr<<"numNeg=0!\n";
    }
    
    //----- SVM
    
    svm_problem problem;
    int indData;
    problem.l= numPos + numNeg;
    problem.y= new double[problem.l];
    problem.x= new svm_node*[problem.l];
    
//     std::cout<<numPos<<" "<<posData.size()<<" "<<numNeg<<"\n";
    
    // add positives
    indData= 0;
    for (std::vector<BoWtype>::iterator itPos= posData.begin();
         itPos!=posData.end();
         ++itPos, ++indData){
        represent::weight( *itPos, idf );
        represent::l2Normalize( *itPos );
        svmUtil::addSVMData( problem, wordToDim, indData, true, *itPos );
    }
    
    // add negatives
    for (std::map<uint32_t,BoWtype>::iterator itNeg= negData.begin();
         itNeg!=negData.end();
         ++itNeg, ++indData){
        represent::weight( itNeg->second, idf );
        represent::l2Normalize( itNeg->second );
        svmUtil::addSVMData( problem, wordToDim, indData, false, itNeg->second );
    }
    
    //----- SVM: params
    
    svm_parameter params;
    params.svm_type= C_SVC;
    params.kernel_type= LINEAR;
    params.C= 1.0;
    
    // defaults from svm-train.c / potentially irrelevant but used in svm.cpp
    params.degree= 3;
    params.gamma= 0;
    params.coef0= 0;
    params.cache_size= 100;
    params.eps= 1e-3;
    params.shrinking= 1;
    params.probability= 0;
    params.nr_weight= 2;
    params.weight_label= (int*)malloc(sizeof(int)*2);
    params.weight_label[0]= +1;
    params.weight_label[1]= -1;
    params.weight= (double*)malloc(sizeof(double)*2);
    params.weight[0]= 1.0;
    params.weight[1]= 1.0;
    
    w.clear();
    const char *error= svm_check_parameter( &problem, &params );
    bool allOK= (error==NULL);
    if (allOK) {
        
        // train
        svm_model *model= svm_train( &problem, &params );
        
        // extract w and b
        b= - model->rho[0];
        int indDim, j;
        for (int iSV= 0; iSV < model->l; ++iSV){
            for (j=0, indDim= model->SV[iSV][0].index;
                 indDim>-1;
                 ++j, indDim= model->SV[iSV][j].index){
                
                w[ dimToWord[indDim] ]+= (model->SV[iSV][j].value) * (model->sv_coef[0][iSV]);
                
            }
        }
        
        // cleanup
        svm_free_and_destroy_model( &model );
        
    } else {
        std::cerr << "LIBSVM check parameters error: "<< error <<"\n";
    }
    
    //----- SVM: cleanup
    
    svm_destroy_param( &params );
    
    for (indData= 0; indData < problem.l; ++indData)
        delete []problem.x[indData];
    delete []problem.y;
    delete []problem.x;
    
    return allOK;
        
}
