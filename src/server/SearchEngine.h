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
#include <fstream>
#include <streambuf>
#include <map>
#include <set>
#include <algorithm>

#include <boost/filesystem.hpp>  // to query/update filesystem

#include <Magick++.h>            // to transform images

#include "ViseMessageQueue.h"

// defined in src/vise.cc
// a global message queue to send communications to client HTTP browser
extern ViseMessageQueue vise_message_queue_;

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
  void InitEngineResources();
  void UpdateEngineOverview();

  void Preprocess();
  void Descriptor();

  void MoveToNextState();
  void MoveToPrevState();

  // getters and setters
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
  void SetEngineConfigParam(std::string key, std::string value);
  std::string GetEngineConfigParam(std::string key);
  void PrintEngineConfig();
  std::string GetEngineOverview();

 private:
  boost::filesystem::path basedir_;
  boost::filesystem::path enginedir_;

  boost::filesystem::path original_imgdir_;
  boost::filesystem::path transformed_imgdir_;
  boost::filesystem::path imglist_fn_;
  std::vector< std::string > imglist_;

  boost::filesystem::path training_datadir_;
  boost::filesystem::path tmp_datadir_;

  // to maintain state of the search engine
  STATE state_;
  std::vector< std::string > state_name_list_;
  std::vector< std::string > state_info_list_;

  std::string engine_name_;

  std::map< std::string, std::string > engine_config_;
  boost::filesystem::path engine_config_fn_;

  bool update_engine_overview_;
  std::ostringstream engine_overview_;

  void CreateEngine( std::string name );
  void LoadEngine( std::string name );
  bool EngineExists( std::string name );
  bool EngineConfigExists();

  void CreateFileList(boost::filesystem::path dir,
                      std::vector<std::string> &filelist);

  void SendMessage(std::string message);
  void SendStatus(std::string status);
  void SendControl(std::string control);
  void SendPacket(std::string sender, std::string type, std::string status);

  void WriteImageListToFile(const std::string fn,
                            const std::vector< std::string > &imlist);
  void WriteConfigToFile();

  void InitEngineResources( std::string name );
};

#endif /* _VISE_SEARCH_ENGINE_H */
