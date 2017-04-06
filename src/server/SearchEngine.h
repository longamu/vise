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
#include <streambuf>
#include <map>

#include <boost/filesystem.hpp>

class SearchEngine {
public:
  enum STATE {
    UNKNOWN = 0,
    INIT,
    CONFIG,
    TRAIN,
    QUERY,
    ERROR
  };

  SearchEngine();
  void Init(std::string name, boost::filesystem::path basedir);
  std::string MoveToNextState();

  std::string Name() {
    return engine_name_;
  }

  boost::filesystem::path GetEngineConfigPath() {
    return engine_config_fn_;
  }
  std::string GetResourceUri(std::string resource_name);

private:
  boost::filesystem::path basedir_;
  boost::filesystem::path enginedir_;
  boost::filesystem::path engine_config_fn_;
  std::string engine_name_;
  std::string engine_config;
  STATE state_;

  void CreateEngine( std::string name );
  void LoadEngine( std::string name );
  bool EngineExists( std::string name );
  bool EngineConfigExists();
};

#endif /* _VISE_SEARCH_ENGINE_H */
