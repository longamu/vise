/** @file   search_engine.h
 *  @brief  Interface to a search engine indexed using VISE
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   01 July 2018
 */

#ifndef _VISE_RELJA_RETRIVAL_H_
#define _VISE_RELJA_RETRIVAL_H_

#include "vise/search_engine.h"

#include <string>

// for filesystem i/o
#include <boost/filesystem.hpp>

// for logging
#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>

// for config file i/o
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

// used by relja_retrival
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

// relja_retrival
#include <fastann.hpp>
#include "dataset_v2.h"        // for datasetAbs
#include "slow_construction.h" // for sequentialConstructions
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "feat_getter.h"
#include "feat_standard.h"
#include "clst_centres.h"
#include "soft_assigner.h"
#include "hamming.h"
#include "hamming_embedder.h"
#include "tfidf_data.pb.h"
#include "tfidf_v2.h"
#include "spatial_verif_v2.h"
#include "mq_filter_outliers.h"

using namespace std;

namespace vise {
  class relja_retrival: public search_engine {
    std::string search_engine_id_;

    // config
    boost::property_tree::ptree config_;
    void load_config();
    void save_config();
    void set_default_config();
    void set_config(const std::string name, const std::string value);

    // resources
    boost::filesystem::path data_dir_;
    boost::filesystem::path vise_data_dir_;
    boost::filesystem::path temp_dir_;

    // state variable
    bool is_search_engine_loaded_;

    // resources
    boost::filesystem::path dset_fn_;
    boost::filesystem::path clst_fn_;
    boost::filesystem::path iidx_fn_;
    boost::filesystem::path fidx_fn_;
    boost::filesystem::path wght_fn_;
    boost::filesystem::path hamm_fn_;
    boost::filesystem::path assign_fn_;
    boost::filesystem::path image_dir_;
    boost::filesystem::path config_fn_;
    boost::filesystem::path imlist_fn_;

    // search engine data structures
    datasetAbs *dataset_;
    sequentialConstructions *cons_queue_;
    protoDbFile *dbFidx_file_;
    protoDbFile *dbIidx_file_;
    protoDbInRamStartDisk *dbFidx_;
    protoDbInRamStartDisk *dbIidx_;
    protoIndex *fidx_;
    protoIndex *iidx_;
    featGetter *feat_getter_;
    clstCentres *clst_centres_;
    fastann::nn_obj<float> *nn_;
    bool use_hamm_;
    softAssigner *soft_assigner_;
    hamming *hamming_emb_;
    embedderFactory *emb_factory_;
    tfidfV2 *tfidf_;
    retrieverFromIter *base_retriever_;
    spatialVerifV2 *spatial_verif_v2_;
    spatialRetriever *spatial_retriever_;
    multiQuery *multi_query_;
    multiQueryMax *multi_query_max_;

  public:
    relja_retrival( const std::string search_engine_id, const boost::filesystem::path data_dir );
    std::string id();
    bool init();
    bool is_load_possible();
    bool load();
    bool unload();
    bool query();
    bool index();


    static std::string get_search_engine_id(std::string name, std::string version) {
      return name + "/" + version;
    }
  };
}
#endif
