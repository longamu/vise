/** @file   search_engine.h
 *  @brief  Interface to a search engine indexed using VISE
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   01 July 2018
 */

#ifndef _VISE_SEARCH_ENGINE_H_
#define _VISE_SEARCH_ENGINE_H_

#include <string>

// for filesystem i/o
#include <boost/filesystem.hpp>

// for logging
#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>

// relja_retrival
#include "dataset_v2.h"

using namespace std;

namespace vise {
  class search_engine {
    std::string search_engine_id_;

    // resources
    boost::filesystem::path dset_fn_;
    boost::filesystem::path clst_fn_;
    boost::filesystem::path iidx_fn_;
    boost::filesystem::path fidx_fn_;
    boost::filesystem::path wght_fn_;
    boost::filesystem::path image_dir_;

    // state variable
    bool is_search_engine_loaded_;

    // search engine data structures
    datasetAbs *dataset_;

  public:
    search_engine(const std::string search_engine_id,
		  const boost::filesystem::path dset_fn,
		  const boost::filesystem::path clst_fn,
		  const boost::filesystem::path iidx_fn,
		  const boost::filesystem::path fidx_fn,
		  const boost::filesystem::path wght_fn,
		  const boost::filesystem::path image_dir
		  );
  
    bool is_possible_to_load_now();
    bool load();
    bool unload();
    bool query();

    static std::string get_search_engine_id(std::string name, std::string version) {
      return name + "/" + version;
    }
  };
}
#endif
