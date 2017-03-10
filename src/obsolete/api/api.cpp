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

#include "spatial_api.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <stdexcept>
#include <string>

#include <boost/bind.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <fastann/fastann.hpp>

#include "python_cfg_to_ini.h"
#include "util.h"
#include "forward_index.h"
#include "fidx_wrapper_jp_db5.h"
#include "iidx_wrapper_jp_gen.h"
#include "iidx_in_ram.h"
#include "fidx_in_ram.h"
#include "slow_construction.h"
#include "spatial_verif.h"
#include "tfidf.h"
#include "quant_desc.h"
#include "clst_centres.h"
#include "feat_standard.h"
#include "soft_assigner.h"
#include "feat_getter.h"
#include "proto_db_file.h"
#include "fidx_v2_as_v1.h"
#include "iidx_v2_as_v1.h"



int main(int argc, char* argv[]){
    
    
    int APIport= 35200;
    if (argc>1){
        APIport= atoi(argv[1]);
    }
    
    std::string dsetname= "oxMini20";
    if (argc>2){
        dsetname= argv[2];
    }
    
    std::string configFn= "~/Relja/Code/relja_retrieval/src/ui/web/config/config.cfg";
    if (argc>3){
        configFn= argv[3];
    }
    configFn= util::expandUser(configFn);
    std::string tempConfigFn= util::getTempFileName();
    pythonCfgToIni( configFn, tempConfigFn );
    
    
    std::cout<<"APIport= "<<APIport<<"\n";
    
    
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(tempConfigFn, pt);
    
    std::string dsetFn= util::expandUser(pt.get<std::string>( dsetname+".dsetFn" ));
    std::string dsetPrefix= pt.get<std::string>( dsetname+".dsetPrefix", dsetname ); // TODO: remove prefix
    std::string docMapFn= util::expandUser(pt.get<std::string>( dsetname+".docMapFn_api" ));
    boost::optional<std::string> clstFn= pt.get_optional<std::string>( dsetname+".clstFn" );
    std::string iidxFn= util::expandUser(pt.get<std::string>( dsetname+".iidxFn" ));
    std::string fidxFn= util::expandUser(pt.get<std::string>( dsetname+".fidxFn" ));
    std::string wghtFn= util::expandUser(pt.get<std::string>( dsetname+".wghtFn" ));
    
    std::string docMapFindPath= pt.get<std::string>( dsetname+".docMapFindPath", "" );
    std::string docMapReplacePath= pt.get<std::string>( dsetname+".docMapReplacePath", "");
    
    bool useRootSIFT= pt.get<bool>(dsetname+".RootSIFT", true);
    
    remove(tempConfigFn.c_str());
    
    if ( !boost::filesystem::exists( docMapFn ) ){
        documentMap docMap( dsetFn.c_str(), dsetPrefix.c_str() );
        docMap.saveToFile( docMapFn.c_str() );
    }
    
    dataset dset( dsetFn.c_str(), dsetPrefix.c_str(), docMapFindPath.c_str(), docMapReplacePath.c_str() ); // needed for register
    documentMap docMap( docMapFn.c_str(), &dset );
    
    std::cout<<docMap.getFn( 0 )<<"\n";;
    
    
    
    sequentialConstructions *consQueue= new sequentialConstructions(); // don't forget to start it
    
    // Set up inverted index
    
    invertedIndex *iidx_obj= NULL;
    protoDbFile const *iidxDbFile= NULL;
    protoDb const *iidxDb= NULL;
    protoDbInRamStartDisk const *iidxDbStart= NULL;
    
    if (std::string(iidxFn.begin() + iidxFn.size() - 5, iidxFn.end()) != "v2bin"){
        
        if (false)
            iidx_obj= new iidxWrapperJpGen<uint32_t>( iidxFn.c_str() );
        else {
            #if 0
            iidx_obj= new iidxInRam( iidxFn.c_str() );
            #else
            
            invertedIndex *iidxOnDisk= new iidxWrapperJpGen<uint32_t>( iidxFn.c_str() );
            boost::function<invertedIndex*()> iidxInRamConstructor= boost::lambda::bind( boost::lambda::new_ptr<iidxInRam>(), boost::lambda::make_const(iidxFn.c_str()) );
            
            iidx_obj= new iidxInRamStartDisk( *iidxOnDisk, iidxInRamConstructor, true, consQueue );
            #endif
        }
        
    } else {
        
        // from v2
        
        iidxDbFile= new protoDbFile(iidxFn);
        
        #if 0
        iidxDb= new protoDbInRam(*iidxDbFile);
        #else
        boost::function<protoDb*()> iidxInRamConstructor= boost::lambda::bind(
            boost::lambda::new_ptr<protoDbInRam>(),
            boost::cref(*iidxDbFile) );
        
        iidxDbStart= new protoDbInRamStartDisk( *iidxDbFile, iidxInRamConstructor, true, consQueue );
        #endif
        
        iidx_obj= new iidxV2AsV1( *iidxDbStart, docMap.numDocs() );
        
    }
    
    // Set up forward index
    
    forwardIndex *fidxDB_obj= NULL;
    protoDbFile const *fidxDbFile= NULL;
    protoDb const *fidxDb= NULL;
    protoDbInRamStartDisk const *fidxDbStart= NULL;
    
    if (std::string(fidxFn.begin() + fidxFn.size() - 5, fidxFn.end()) != "v2bin"){
        
        if (false)
            fidxDB_obj= new fidxWrapperJpDb5_HardJP( fidxFn.c_str(), docMap );
        else {
            #if 0
            fidxWrapperJpDb5_HardJP fidx_obj_orig( fidxFn.c_str(), docMap );
            fidxDB_obj= new fidxInRam(fidx_obj_orig);
            #else
            
            forwardIndex *fidxOnDisk= new fidxWrapperJpDb5_HardJP( fidxFn.c_str(), docMap );
            boost::function<forwardIndex*()> fidxInRamConstructor= boost::lambda::bind( boost::lambda::new_ptr<fidxInRam>(), boost::cref(*fidxOnDisk) );
            
            fidxDB_obj= new fidxInRamStartDisk( *fidxOnDisk, fidxInRamConstructor, true, consQueue );
            #endif
        }
        
    } else {
        
        // from v2
        
        fidxDbFile= new protoDbFile(fidxFn);
        
        #if 0
        fidxDb= new protoDbInRam(*fidxDbFile);
        #else
        
        boost::function<protoDb*()> fidxInRamConstructor= boost::lambda::bind(
            boost::lambda::new_ptr<protoDbInRam>(),
            boost::cref(*fidxDbFile) );
        
        fidxDbStart= new protoDbInRamStartDisk( *fidxDbFile, fidxInRamConstructor, true, consQueue );
        #endif
        
        fidxDB_obj= new fidxV2AsV1( *fidxDbStart, docMap.numDocs() );
        
    }
    
    // start the construction of inRam stuff
    consQueue->start();
    
    
    forwardIndex *fidxQ_obj= fidxDB_obj;
    
    bool const softQ= true;
    
    featGetter *featGetter_obj= NULL;
    clstCentres *clstCentres_obj= NULL;
    fastann::nn_obj<float> const *nn_obj= NULL;
    softAssigner *SA_obj= NULL;
    
    if (clstFn.is_initialized()){
        
        // feature getter
        bool SIFTscale3= pt.get<bool>( dsetname+".SIFTscale3", true);
        featGetter_obj= new featGetter_standard( (
            std::string("hesaff-") +
            std::string((useRootSIFT ? "rootsift" : "sift")) +
            std::string(SIFTscale3 ? "-scale3" : "")
            ).c_str() );
        
        // clusters
        std::cout<<"api::main: Loading cluster centres\n";
        double t0= timing::tic();
        clstCentres_obj= new clstCentres( util::expandUser(*clstFn).c_str(), true );
        std::cout<<"api::main: Loading cluster centres - DONE ("<< timing::toc(t0) <<" ms)\n";
        
        std::cout<<"api::main: Constructing NN search object\n";
        t0= timing::tic();
        
        nn_obj=
        #if 1
            fastann::nn_obj_build_kdtree(
                clstCentres_obj->clstC_flat,
                clstCentres_obj->numClst,
                clstCentres_obj->numDims, 8, 1024);
        #else
            fastann::nn_obj_build_exact(
                clstCentres_obj->clstC_flat,
                clstCentres_obj->numClst,
                clstCentres_obj->numDims);
        #endif
        std::cout<<"api::main: Constructing NN search object - DONE ("<< timing::toc(t0) << " ms)\n";
        
        // soft assigner
        if (softQ) {
            if (useRootSIFT)
                SA_obj= new SA_exp( 0.02 );
            else
                SA_obj= new SA_exp( 6250 );
        }
    }
    
    if ( !boost::filesystem::exists( wghtFn.c_str() ) ){
        tfidf tfidf_obj( iidx_obj, fidxDB_obj );
        tfidf_obj.saveToFile( wghtFn.c_str() );
    }
    tfidf tfidf_obj( iidx_obj, fidxQ_obj, wghtFn.c_str(), featGetter_obj, nn_obj, SA_obj );
    
    spatParams spatParams_obj(200,4);
    spatialVerif spatialVerif_obj( &tfidf_obj, fidxDB_obj, &spatParams_obj, false );
    multiQuery *mq= new multiQueryMax( spatialVerif_obj );
    
    documentMapAsDatasetAbs dsetV2Wrapper(docMap);
    API API_obj( spatialVerif_obj, mq, dsetV2Wrapper );
    
    boost::asio::io_service io_service;
    API_obj.server(io_service, APIport);
    
    // make sure this is deleted before everything which uses it
    delete consQueue;
    
    delete iidx_obj;
    delete fidxDB_obj;
    delete mq;
    if (clstCentres_obj!=NULL){
        delete nn_obj;
        delete clstCentres_obj;
        delete featGetter_obj;
    }
    if (SA_obj!=NULL)
        delete SA_obj;
    
    if (iidxDbFile!=NULL)
        delete iidxDbFile;
    if (iidxDb!=NULL)
        delete iidxDb;
    if (iidxDbStart!=NULL)
        delete iidxDbStart;
    
    if (fidxDbFile!=NULL)
        delete fidxDbFile;
    if (fidxDb!=NULL)
        delete fidxDb;
    if (fidxDbStart!=NULL)
        delete fidxDbStart;
    
    return 0;
    
}
