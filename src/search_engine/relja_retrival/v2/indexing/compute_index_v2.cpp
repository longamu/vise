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

#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <google/protobuf/stubs/common.h>

#include "build_index.h"
#include "embedder.h"
#include "feat_standard.h"
#include "hamming_embedder.h"
#include "mpi_queue.h"
#include "python_cfg_to_ini.h"
#include "train_assign.h"
#include "train_descs.h"
#include "train_hamming.h"
#include "util.h"


int main(int argc, char* argv[]){
    
    MPI_INIT_ENV
    MPI_GLOBAL_ALL
    
    if (rank==0)
        std::cout<<"numProc= "<<numProc<<"\n";
    
    // ------------------------------------ setup config
    
    std::string stage= "index";
    if (argc>1) stage= argv[1];
    std::string dsetname= "oxMini20_v2";
    if (argc>2) dsetname= argv[2];
    std::string configFn= "../src/ui/web/config/config.cfg";
    if (argc>3) configFn= argv[3];
    
    configFn= util::expandUser(configFn);
    std::string tempConfigFn= util::getTempFileName();
    pythonCfgToIni( configFn, tempConfigFn );
    
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(tempConfigFn, pt);
    
    // ------------------------------------ read config
    
    std::string const clstFn= util::expandUser(pt.get<std::string>( dsetname+".clstFn" ));
    uint32_t const vocSize= pt.get<uint32_t>( dsetname+".vocSize" );
    
    bool const useRootSIFT= pt.get<bool>(dsetname+".RootSIFT", true);
    bool const SIFTscale3= pt.get<bool>( dsetname+".SIFTscale3", true);
    
    
    if (stage=="trainDescs"){
        // ------------------------------------ compute training descriptors (i.e. for clustering)
        
        // feature getter
        // NOTE: regardless of RootSIFT, extract SIFT as requires 4 times less storage, can convert later
        featGetter_standard const featGetter_obj( (
                std::string("hesaff-") +
                "sift" +
                std::string(SIFTscale3 ? "-scale3" : "")
                ).c_str() );
        
        std::string const imagelistFn= util::expandUser(pt.get<std::string>( dsetname+".imagelistFn", "" ));
        std::string const trainImagelistFn= util::expandUser(pt.get<std::string>( dsetname+".trainImagelistFn", imagelistFn));
        std::string const databasePath= util::expandUser(pt.get<std::string>( dsetname+".databasePath", "" ));
        std::string const trainDatabasePath= util::expandUser(pt.get<std::string>( dsetname+".trainDatabasePath", databasePath));
        int32_t trainNumDescs= pt.get<int32_t>( dsetname+".trainNumDescs", -1 );
        std::string const trainFilesPrefix= util::expandUser(pt.get<std::string>( dsetname+".trainFilesPrefix" ));
        std::string const trainDescsFn= trainFilesPrefix+"descs.e3bin";
        
        buildIndex::computeTrainDescs(
            trainImagelistFn, trainDatabasePath,
            trainDescsFn,
            trainNumDescs,
            featGetter_obj);
        
    } else if (stage=="trainAssign"){
        // ------------------------------------ assign training descs to clusters
        
        std::string const trainFilesPrefix= util::expandUser(pt.get<std::string>( dsetname+".trainFilesPrefix" ));
        std::string const trainDescsFn= trainFilesPrefix+"descs.e3bin";
        std::string const trainAssignsFn= trainFilesPrefix + util::uintToShortStr(vocSize) + "_assigns.bin";
        
        buildIndex::computeTrainAssigns( clstFn, useRootSIFT, trainDescsFn, trainAssignsFn);
        
    } else if (stage=="trainHamm"){
        // ------------------------------------ compute hamming stuff
        // (i.e. rotation, medians)
        
        uint32_t const hammEmbBits= pt.get<uint32_t>( dsetname+".hammEmbBits" );
        std::string const trainFilesPrefix= util::expandUser(pt.get<std::string>( dsetname+".trainFilesPrefix" ));
        std::string const trainDescsFn= trainFilesPrefix+"descs.e3bin";
        std::string const trainAssignsFn= trainFilesPrefix + util::uintToShortStr(vocSize) + "_assigns.bin";
        std::string const trainHammFn= trainFilesPrefix + util::uintToShortStr(vocSize) + "_hamm" + boost::lexical_cast<std::string>(hammEmbBits) + ".v2bin";
        
        buildIndex::computeHamming(clstFn, useRootSIFT, trainDescsFn, trainAssignsFn, trainHammFn, hammEmbBits);
        
    } else if (stage=="index"){
        // ------------------------------------ compute index
        
        std::string const imagelistFn= util::expandUser(pt.get<std::string>( dsetname+".imagelistFn" ));
        std::string const databasePath= util::expandUser(pt.get<std::string>( dsetname+".databasePath" ));
        boost::optional<uint32_t> const hammEmbBits= pt.get_optional<uint32_t>( dsetname+".hammEmbBits" );
        std::string const dsetFn= util::expandUser(pt.get<std::string>( dsetname+".dsetFn" ));
        std::string const iidxFn= util::expandUser(pt.get<std::string>( dsetname+".iidxFn" ));
        std::string const fidxFn= util::expandUser(pt.get<std::string>( dsetname+".fidxFn" ));
        std::string const tmpDir= util::expandUser(pt.get<std::string>( dsetname+".tmpDir" ));
        
        // feature getter
        featGetter_standard const featGetter_obj( (
                std::string("hesaff-") +
                std::string((useRootSIFT ? "rootsift" : "sift")) +
                std::string(SIFTscale3 ? "-scale3" : "")
                ).c_str() );
        
        // embedder
        embedderFactory *embFactory= NULL;
        if (hammEmbBits.is_initialized()){
            std::string const trainFilesPrefix= util::expandUser(pt.get<std::string>( dsetname+".trainFilesPrefix" ));
            std::string const trainHammFn= trainFilesPrefix + util::uintToShortStr(vocSize) + "_hamm" + boost::lexical_cast<std::string>(*hammEmbBits) + ".v2bin";
            
            embFactory= new hammingEmbedderFactory(trainHammFn, *hammEmbBits);
        }
        else
            embFactory= new noEmbedderFactory;
    //     rawEmbedderFactory embFactory(featGetter_obj.numDims());
        
        buildIndex::build(imagelistFn, databasePath,
                          dsetFn,
                          iidxFn,
                          fidxFn,
                          tmpDir,
                          featGetter_obj,
                          clstFn,
                          embFactory );
        
        delete embFactory;
    } else {
        throw std::runtime_error( std::string("Unrecognized stage: ") + stage);
    }
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
