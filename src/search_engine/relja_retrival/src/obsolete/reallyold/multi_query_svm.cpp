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

#include "multi_query_svm.h"

#include <algorithm>

#define ESVM_CALIB_PLATT
#ifndef ESVM_CALIB_PLATT
// for linear ESVM calibration
#include <cv.h>
#else
#include "platt_sigmoidfit.h"
#endif

#include "util.h"
#include "svm.h"
#include "svm_util.h"
#include "represent.h"
#include "weighter.h"
#include "ellipse.h"
#include "quant_desc.h"
#include "represent.h"



void
negDataSVMHelper( forwardIndex const &forwardIndex_obj, uint32_t numNeg, std::vector< std::map<uint32_t,double> > *negatives= NULL, std::set<uint32_t> *negDocIDs= NULL, uint32_t seed= 43 ){
    
    bool delDocIDs= (negDocIDs==NULL);
    if (delDocIDs){
        negDocIDs= new std::set<uint32_t>();
    }
    negDocIDs->clear();
    
    if (negatives!=NULL)
        negatives->resize( numNeg );
    
    // not thread safe as the random number generator has an internal state, if you want thread safe look up det_ransac.cpp for boost
    std::srand(seed);
    // TODO implement better as this is a bit stupid and inefficient, only ok for numNeg << numDocs
    uint32_t numDocs= forwardIndex_obj.numDocs();
    
    uint32_t docID;
    
    while( negDocIDs->size()!=numNeg ){
        
        docID= std::rand() % numDocs;
        
        if (negDocIDs->count( docID )==0){
            
            negDocIDs->insert( docID );
            if (negatives==NULL)
                continue;
            
            std::vector<quantDesc> words;
            std::vector<ellipse> regions;
            forwardIndex_obj.getWordsRegs( docID, words, regions );
            represent::computeBoW( words, negatives->at( negDocIDs->size()-1 ) );
            
        }
    }
    
    if (delDocIDs){
        delete negDocIDs;
    }
    
}



multiQueryJointSVM::multiQueryJointSVM( tfidf const &aTfidf_obj, forwardIndex const &forwardIndex_obj ) : multiQuery(), tfidf_obj(&aTfidf_obj) {
    
    negDataSVMHelper( forwardIndex_obj, numNeg, &negatives, NULL, 43 );
    
}



void
multiQueryJointSVM::query( retsType &represent_objs, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    
    std::set< uint32_t > wordsUsed;
    
    //----- positive training data
    
    for (retsType::iterator itRep= represent_objs.begin(); itRep!=represent_objs.end(); ++itRep){
        
        for (std::map<uint32_t,double>::iterator itBe= (*itRep)->BoW.begin(); itBe!=(*itRep)->BoW.end(); ++itBe)
            wordsUsed.insert( itBe->first );
        
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
    
    
    //----- SVM
    
    uint32_t numPos= represent_objs.size();
    
    svm_problem problem;
    int indData= 0;
    problem.l= numPos + numNeg;
    problem.y= new double[problem.l];
    problem.x= new svm_node*[problem.l];
    
    // add positives
    for (retsType::iterator itRep= represent_objs.begin(); itRep!=represent_objs.end(); ++itRep, ++indData){
        BoWtype BowPos= (*itRep)->BoW;
        represent::weight( BowPos, tfidf_obj->idf );
        represent::l2Normalize( BowPos );
        svmUtil::addSVMData( problem, wordToDim, indData, true, BowPos );
    }
    
    // add negatives
    for (std::vector<BoWtype>::const_iterator itNeg= negatives.begin(); itNeg!=negatives.end(); ++itNeg, ++indData){
        
        BoWtype BoWtrunc;
        for (BoWtype::const_iterator itB= itNeg->begin(); itB!=itNeg->end(); ++itB)
            if (wordsUsed.count(itB->first))
                BoWtrunc[ itB->first ]= itB->second;
        
        represent::weight( BoWtrunc, tfidf_obj->idf );
        represent::l2Normalize( BoWtrunc );
        svmUtil::addSVMData( problem, wordToDim, indData, false, BoWtrunc );
    }
    
    
    
    //----- SVM: params
    // adapted from query_expand_svm TODO unify
    
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
    
    std::map<uint32_t,double> w;
    double b= 0;
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
    
    
    //----- query with learnt weights
    
    std::vector<double> scores;
    weighter::queryExecute( w, tfidf_obj, scores, b );
    util::normTo0_1( scores );
    retriever::sortResults( scores, queryRes, toReturn );
    
}



exemplarSVM::exemplarSVM( tfidf const &aTfidf_obj, forwardIndex *aForwardIndex_obj, spatialVerif const *aSpatVerif_obj, bool aDoCalibrate ) : bowRetriever(aForwardIndex_obj), tfidf_obj(&aTfidf_obj), spatVerif_obj(aSpatVerif_obj), doCalibrate(aDoCalibrate) {
    
    std::set<uint32_t> negDocIDs;
    negDataSVMHelper( *forwardIndex_obj, numNeg, &negatives, NULL, 43 );
    if (doCalibrate)
        negDataSVMHelper( *forwardIndex_obj, numCalibNeg, NULL, &calibNegDocIDs, 44 );
    
}



void
exemplarSVM::query( represent &represent_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    
    // if we're doing calibration then we want all results in order to get scores for negatives
    uint32_t origToReturn= toReturn;
    if (spatVerif_obj!=NULL && doCalibrate)
        toReturn= 0;
    
    std::set< uint32_t > wordsUsed;
    
    //----- positive training data
    
    BoWtype BowPos= represent_obj.BoW;
    
    for (std::map<uint32_t,double>::iterator itBe= BowPos.begin(); itBe!=BowPos.end(); ++itBe)
        wordsUsed.insert( itBe->first );
    
    
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
    
    
    //----- SVM
    
    uint32_t numPos= 1;
    
    svm_problem problem;
    int indData= 0;
    problem.l= numPos + numNeg;
    problem.y= new double[problem.l];
    problem.x= new svm_node*[problem.l];
    
    // add positives
    represent::weight( BowPos, tfidf_obj->idf );
    represent::l2Normalize( BowPos );
    svmUtil::addSVMData( problem, wordToDim, indData, true, BowPos );
    ++indData;
    
    // add negatives
    for (std::vector<BoWtype>::const_iterator itNeg= negatives.begin(); itNeg!=negatives.end(); ++itNeg, ++indData){
        
        BoWtype BoWtrunc;
        for (BoWtype::const_iterator itB= itNeg->begin(); itB!=itNeg->end(); ++itB)
            if (wordsUsed.count(itB->first))
                BoWtrunc[ itB->first ]= itB->second;
        
        represent::weight( BoWtrunc, tfidf_obj->idf );
        represent::l2Normalize( BoWtrunc );
        svmUtil::addSVMData( problem, wordToDim, indData, false, BoWtrunc );
    }
    
    
    
    //----- SVM: params
    // adapted from query_expand_svm TODO unify
    
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
    
    std::map<uint32_t,double> w;
    double b= 0;
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
    
    
    //----- query with learnt weights
    
    std::vector<double> scores;
    weighter::queryExecute( w, tfidf_obj, scores, b );
    util::normTo0_1( scores );
    retriever::sortResults( scores, queryRes, toReturn );
    
    // do spatial verification + calibration based on it
    
    if (spatVerif_obj!=NULL){
        
        spatWorker worker( represent_obj, *spatVerif_obj, queryRes );
        spatManager manager( queryRes );
        
        threadQueue<double>::start(
            std::min( static_cast<uint32_t>( queryRes.size() ), spatVerif_obj->spatialDepth() ),
            worker, manager, 4
        );
        
        retriever::sortResults( queryRes, spatVerif_obj->spatialDepth(), toReturn );
        
        if (doCalibrate){
            
            double scale= 0.001; // decrease scores if no spatially verified results
            double shift= 0;
            
            // positives
            
            std::vector<double> posScores;
            std::set<uint32_t> posDocIDs;
            
            uint32_t numVerif;
            for (numVerif= 0; numVerif < queryRes.size() && queryRes[numVerif].second > scoreThr; ++numVerif){
                posScores.push_back( scores[ queryRes[numVerif].first ] );
                posDocIDs.insert( queryRes[numVerif].first );
            }
            
            if (numVerif>0){
                
                std::vector<double> calibNegScores;
                
                for (std::set<uint32_t>::const_iterator negIt=calibNegDocIDs.begin(); negIt!=calibNegDocIDs.end(); ++negIt)
                    // make sure a positive is not in "negatives"
                    if (posDocIDs.count(*negIt)==0){
                        calibNegScores.push_back( scores[*negIt] );
                    }
                
                #ifdef ESVM_CALIB_PLATT
                    
                    std::vector<double> deci;
                    std::vector<bool> label;
                    deci.reserve( posScores.size() + calibNegScores.size() );
                    label.reserve( posScores.size() + calibNegScores.size() );
                    for (std::vector<double>::const_iterator it= posScores.begin(); it!=posScores.end(); ++it){
                        deci.push_back( *it );
                        label.push_back( true );
                    }
                    for (std::vector<double>::const_iterator it= calibNegScores.begin(); it!=calibNegScores.end(); ++it){
                        deci.push_back( *it );
                        label.push_back( false );
                    }
                    
                    platt_sigmoidfit( deci, label, scale, shift );
                    
                #else
                    // instead of sigmoid, as it needs gradient descent, minimize (scale*score+shift-label)^2, which has a closed form solution; label=1 for positive, 0 for negative
                    
                    // set up the least squares
                    
                    uint32_t numPos= posScores.size();
                    cv::Mat A( numPos+calibNegScores.size(), 2, cv::DataType<double>::type ), y( numPos+calibNegScores.size(), 1, cv::DataType<double>::type), x;
                    
                    for (uint32_t i= 0; i<numPos; ++i){
                        A.at<double>(i,0)= posScores[i];
                        A.at<double>(i,1)= 1.0;
                        y.at<double>(i,0)= 1.0;
                    }
                    
                    for (uint32_t i= 0; i<calibNegScores.size(); ++i){
                        A.at<double>(i+numPos,0)= calibNegScores[i];
                        A.at<double>(i+numPos,1)= 1.0;
                        y.at<double>(i+numPos,0)= 0.0;
                    }
                    
                    cv::solve(A, y, x, cv::DECOMP_SVD);
                    
                    scale= x.at<double>(0,0);
                    shift= x.at<double>(1,0);
                    
                #endif
                
            }
            
            #ifndef ESVM_CALIB_PLATT
                // scale and shift the data
                // just "make sure" results are <1
                scale*= 0.01;
                shift*= 0.01;
            #endif
            
            double addedScore;
            uint32_t i_=0;
            
            for (std::vector<indScorePair>::iterator itRes= queryRes.begin(); itRes!=queryRes.end(); ++itRes, ++i_){
                
                addedScore= itRes->second - scores[ itRes->first ];
                
                itRes->second= addedScore + 
                    #ifdef ESVM_CALIB_PLATT
                        1.0/(1.0+ exp( scale* scores[itRes->first] + shift) );
                    #else
                        (scale* scores[itRes->first] + shift);
                    #endif
                
            }
                
            // order could have changed a bit as maybe positives get a score >1
            retriever::sortResults( queryRes, 0, origToReturn );
            
        }
        
    }
    
}
