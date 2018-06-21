/** @file   search_engine_manager.h
 *  @brief  A singleton class which handles http requests related to vise
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   18 June 2018
 */

#ifndef _VISE_SEARCH_ENGINE_MANAGER_H_
#define _VISE_SEARCH_ENGINE_MANAGER_H_

#include <iostream>
#include <sstream>
#include <fstream>

#include <boost/filesystem.hpp>

#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>

// to generate uuid
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <Magick++.h>            // to transform images

#include <eigen3/Eigen/Dense>

#include "http_server/http_request.h"
#include "http_server/http_response.h"

#include "vise/util.h"

using namespace std;
using namespace Eigen;

// uses C++ singleton design pattern
class search_engine_manager {
  boost::filesystem::path data_dir_;  // storage for vise internal data, search engine repo.
  boost::filesystem::path asset_dir_; // location of ui, logo, etc.

  search_engine_manager() { };
  search_engine_manager(const search_engine_manager& sh) { };
  search_engine_manager* operator=(const search_engine_manager &) {
    return 0;
  }

  static search_engine_manager* search_engine_manager_;

  public:
  static search_engine_manager* instance();

  void init(const boost::filesystem::path data_dir);
  void process_cmd(const std::string search_engine_name,
                   const std::string search_engine_version,
                   const std::string search_engine_command,
                   const std::map<std::string, std::string> uri_param,
                   const std::string request_body,
                   http_response& response);

  // search engine data dir management
  boost::filesystem::path convert_to_unique_filename(boost::filesystem::path filename);
  std::string get_unique_filename(std::string extension="");
  bool search_engine_exists(const std::string search_engine_name,
                            const std::string search_engine_version);
  bool create_search_engine(const std::string search_engine_name,
                            const std::string search_engine_version);
  boost::filesystem::path get_search_engine_dir(const std::string search_engine_name,
                                                const std::string search_engine_version);
  boost::filesystem::path get_image_data_dir(const std::string search_engine_name,
                                             const std::string search_engine_version);
  boost::filesystem::path get_vise_data_dir(const std::string search_engine_name,
                                            const std::string search_engine_version);
  boost::filesystem::path get_log_data_dir(const std::string search_engine_name,
                                           const std::string search_engine_version);
  // image i/o
  bool add_image_from_http_payload(const boost::filesystem::path filename,
                                   const std::string& request_body);

};
#endif
