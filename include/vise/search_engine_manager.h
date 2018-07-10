/** @file   search_engine_manager.h
 *  @brief  A singleton class which handles http requests related to vise
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   18 June 2018
 */

#ifndef _VISE_SEARCH_ENGINE_MANAGER_H_
#define _VISE_SEARCH_ENGINE_MANAGER_H_

#include "vise/util.h"
#include "search_engine.h"

#include <iostream>
#include <sstream>
#include <fstream>

// thread
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/process.hpp>

// for config file i/o
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

// for filesystem i/o
#include <boost/filesystem.hpp>

// for logging
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

using namespace std;
using namespace Eigen;

// uses C++ singleton design pattern
class search_engine_manager {

  boost::filesystem::path data_dir_;  // storage for vise internal data, search engine repo.
  boost::filesystem::path asset_dir_; // location of ui, logo, etc.

  bool auto_load_search_engine_;

  static search_engine_manager* search_engine_manager_;

  // threads
  boost::thread* search_engine_index_thread_;
  bool search_engine_index_thread_running_;

  std::string now_search_engine_name_;
  std::string now_search_engine_version_;
  std::map<std::string, std::string> now_search_engine_index_steps_done_;
  std::map<std::string, std::string> now_search_engine_index_steps_count_;
  std::map<std::string, std::string> now_search_engine_index_state_;
  std::vector<std::string> search_engine_index_state_name_list_;
  std::vector<std::string> search_engine_index_state_desc_list_;
  std::map<std::string, vise::search_engine > search_engine_list_;
  
  search_engine_manager() { };
  search_engine_manager(const search_engine_manager& sh) { };
  search_engine_manager* operator=(const search_engine_manager &) {
    return 0;
  }

  bool load_search_engine(std::string search_engine_name, std::string search_engine_version);
  bool unload_search_engine(std::string search_engine_name, std::string search_engine_version);
  bool load_search_engine_check(std::string search_engine_name, std::string search_engine_version);


  public:
  static search_engine_manager* instance();

  void init(const boost::filesystem::path data_dir);
  // POST /vise/admin/_NAME_/_VERSION_/_COMMAND_
  void process_cmd(const std::string search_engine_name,
                   const std::string search_engine_version,
                   const std::string search_engine_command,
                   const std::map<std::string, std::string> uri_param,
                   const std::string request_body,
                   http_response& response);
  // POST /vise/admin/_COMMAND_
  void admin(const std::string command,
             const std::map<std::string, std::string> uri_param,
             const std::string request_body,
             http_response& response);
  // POST /vise/admin/_NAME_/_VERSION_/_COMMAND_
  void query(const std::string search_engine_name,
             const std::string search_engine_version,
             const std::string search_engine_query,
             const std::map<std::string, std::string> uri_param,
             const std::string request_body,
             http_response& response);

  // search engine data dir management
  boost::filesystem::path convert_to_unique_filename(boost::filesystem::path filename);
  std::string get_unique_filename(std::string extension="");
  bool search_engine_exists(const std::string search_engine_name,
                            const std::string search_engine_version);
  bool create_search_engine(const std::string search_engine_name,
                            const std::string search_engine_version,
                            const std::string search_engine_description);
  bool create_default_config(const std::string search_engine_name,
                             const std::string search_engine_version,
                             const std::string search_engine_description);
  bool set_search_engine_config(const std::string search_engine_name,
                                const std::string search_engine_version,
                                const std::string config_name,
                                const std::string config_value);

  boost::filesystem::path get_search_engine_dir(const std::string search_engine_name,
                                                const std::string search_engine_version);
  boost::filesystem::path get_image_data_dir(const std::string search_engine_name,
                                             const std::string search_engine_version);
  boost::filesystem::path get_vise_data_dir(const std::string search_engine_name,
                                            const std::string search_engine_version);
  boost::filesystem::path get_log_data_dir(const std::string search_engine_name,
                                           const std::string search_engine_version);
  boost::filesystem::path get_temp_data_dir(const std::string search_engine_name,
                                            const std::string search_engine_version);
  boost::filesystem::path get_image_filename(const std::string search_engine_name,
                                             const std::string search_engine_version,
                                             const std::map<std::string, std::string>& uri_param );
  boost::filesystem::path get_config_filename(const std::string search_engine_name,
                                              const std::string search_engine_version);
  boost::filesystem::path get_image_list_filename(const std::string search_engine_name,
                                                  const std::string search_engine_version);
  boost::filesystem::path get_index_log_filename(const std::string search_engine_name,
                                                 const std::string search_engine_version);

  // image i/o
  bool add_image_from_http_payload(const boost::filesystem::path filename,
                                   const std::string& request_body);

  bool index_start(const std::string search_engine_name,
                   const std::string search_engine_version);
  void clear_now_search_engine_index_state(void);
  bool run_shell_command(std::string name,
                         std::string cmd);

};
#endif
