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

#include <boost/filesystem.hpp>

class SearchEngine {
public:
  enum STATE {
    UNKNOWN = 0,
    INIT,
    CONFIG,
    TRAIN_UNKNOWN,
    TRAIN_FILELIST,
    TRAIN_DESCS,
    TRAIN_CLUST,
    TRAIN_ASSIGN,
    TRAIN_HAMM,
    TRAIN_INDEX,
    TRAIN_DONE,
    QUERY,
    ERROR
  };

  SearchEngine();
  void Init(std::string name, boost::filesystem::path basedir);
  std::string MoveToNextState();

  STATE GetEngineState() {
    return state_;
  }

  std::string Name() {
    return engine_name_;
  }

  boost::filesystem::path GetEngineConfigPath() {
    return engine_config_fn_;
  }

  std::string GetResourceUri(std::string resource_name);

  void SetEngineConfig(std::string engine_config);
  std::string GetEngineConfigParam(std::string key);
  void PrintEngineConfig();
 private:
  boost::filesystem::path basedir_;
  boost::filesystem::path enginedir_;
  boost::filesystem::path engine_config_fn_;
  std::string engine_name_;
  STATE state_;
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
