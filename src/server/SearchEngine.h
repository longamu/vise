/** @file   search_engine.h
 *  @brief  allow search of an image set
 *
 *  Given a set of images, this module lets initialize, configure and train a
 *  model that allows image region based queries on this image set
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   31 March 2017
 */

#ifndef _VISE_SEARCH_ENGINE_H
#define _VISE_SEARCH_ENGINE_H

#include <iostream>
#include <string>
#include <sstream>
#include <streambuf>
#include <map>
#include <set>
#include <algorithm>

#include <boost/filesystem.hpp>

class SearchEngine {
public:
  enum STATE {
    INITIALIZE=0,
    SETTINGS,
    OVERVIEW,
    PREPROCESSING,
    COMPUTE_DESCRIPTORS,
    CLUSTER_DESCRIPTORS,
    ASSIGN_CLUSTER,
    COMPUTER_HAMM,
    INDEX,
    QUERY,
    STATE_COUNT
  };

  SearchEngine();
  void Init(std::string name, boost::filesystem::path basedir);

  void MoveToNextState();
  STATE GetEngineState();
  std::string GetEngineStateName();
  std::string GetEngineStateName( unsigned int state_id );
  std::string GetEngineStateInfo();
  std::string GetEngineStateList();
  int GetEngineState(std::string state_name);
  std::string Name();
  boost::filesystem::path GetEngineConfigPath();
  std::string GetResourceUri(std::string resource_name);
  std::string GetSearchEngineBaseUri();
  void SetEngineConfig(std::string engine_config);
  std::string GetEngineConfigParam(std::string key);
  void PrintEngineConfig();

 private:
  boost::filesystem::path basedir_;
  boost::filesystem::path enginedir_;
  boost::filesystem::path engine_config_fn_;

  // to maintain state of the search engine
  STATE state_;
  std::vector< std::string > state_name_list_;
  std::vector< std::string > state_info_list_;

  std::string engine_name_;
  std::map< std::string, std::string > engine_config_;

  void CreateEngine( std::string name );
  void LoadEngine( std::string name );
  bool EngineExists( std::string name );
  bool EngineConfigExists();

  void RunTrainingCommand(std::string cmd, std::vector< std::string > cmd_params);
  void CreateFileList(boost::filesystem::path dir,
                      std::set<std::string> acceptable_types,
                      std::ostringstream &filelist);
};

#endif /* _VISE_SEARCH_ENGINE_H */
