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
#include <fstream>
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
    TRAIN_DESCS,
    TRAIN_CLUSTER,
    TRAIN_ASSIGN,
    TRAIN_HAMM,
    TRAIN_INDEX,
    QUERY,
    ERROR
  };

  SearchEngine();
  void Init(std::string name, boost::filesystem::path basedir);
  void MoveToNextState(std::string &result);
private:
  boost::filesystem::path basedir_;
  boost::filesystem::path enginedir_;
  std::string name_;
  STATE state_;
  std::string config_html_;

  void CreateEngine( std::string name );
  void LoadEngine( std::string name );
  bool EngineExists( std::string name );
  void ConfigureEngine(std::string config_json);
  void LoadFile(std::string filename, std::string &file_contents);

  // helper function
  void JsonToMap( std::string json, std::map<std::string, std::string> );
};

#endif /* _VISE_SEARCH_ENGINE_H */
