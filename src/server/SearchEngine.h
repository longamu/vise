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

#include <cstdio>                 // for popen()

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <map>
#include <set>
#include <algorithm>
#include <unistd.h>
#include <locale>                // for std::tolower
#include <cassert>               // for assert()
#include <boost/filesystem.hpp>  // to query/update filesystem

#include <Magick++.h>            // to transform images

#include "ViseMessageQueue.h"

#include "feat_standard.h"
#include "train_descs.h"
#include "train_assign.h"
#include "train_hamming.h"
#include "build_index.h"
#include "hamming_embedder.h"

// defined in src/vise.cc
// a global message queue to send communications to client HTTP browser
extern ViseMessageQueue vise_message_queue_;

class SearchEngine {
public:

  SearchEngine();
  void Init(std::string name, boost::filesystem::path engine_dir );
  void UpdateEngineOverview();

  void Preprocess();
  void Descriptor();
  void Cluster();
  void Assign();
  void Hamm();
  void Index();
  void Query();

  // helper functions to query state of search engine
  std::string GetName();
  bool IsEngineConfigEmpty();
  bool IsImglistEmpty();
  bool EngineConfigFnExists();
  bool ImglistFnExists();
  bool DescFnExists();
  bool ClstFnExists();
  bool AssignFnExists();
  bool HammFnExists();
  bool IndexFnExists();
  std::string GetEngineOverview();
  unsigned long GetImglistOriginalSize();
  unsigned long GetImglistTransformedSize();
  unsigned long DescFnSize();
  unsigned long ClstFnSize();
  unsigned long AssignFnSize();
  unsigned long HammFnSize();
  unsigned long IndexFnSize();

  boost::filesystem::path GetEngineConfigFn();
  std::string GetResourceUri(std::string resource_name);
  std::string GetSearchEngineBaseUri();
  boost::filesystem::path GetOriginalImageDir();
  boost::filesystem::path GetTransformedImageDir();

  void SetEngineConfig(std::string engine_config);
  void SetEngineConfigParam(std::string key, std::string value);
  std::string GetEngineConfigParam(std::string key);
  bool EngineConfigParamExists(std::string key);
  void PrintEngineConfig();

  void LoadImglist();
  bool IsEngineNameValid(std::string engine_name);
  void WriteConfigToFile();
  void ReadConfigFromFile();

 private:
  std::string engine_name_;

  boost::filesystem::path basedir_;
  boost::filesystem::path enginedir_;

  boost::filesystem::path original_imgdir_;
  boost::filesystem::path transformed_imgdir_;
  boost::filesystem::path imglist_fn_;
  std::vector< std::string > imglist_;
  std::vector< unsigned int > imglist_fn_original_size_;
  std::vector< unsigned int > imglist_fn_transformed_size_;

  boost::filesystem::path training_datadir_;
  boost::filesystem::path tmp_datadir_;

  std::map< std::string, std::string > engine_config_;
  boost::filesystem::path engine_config_fn_;

  bool update_engine_overview_;
  std::ostringstream engine_overview_;

  std::set< std::string > acceptable_img_ext_;

  void CreateEngine( std::string name );
  void LoadEngine( std::string name );
  bool EngineConfigExists();

  void SendProgress(std::string state_name, unsigned long completed, unsigned long total);
  void SendCommand(std::string sender, std::string command);
  void SendLog(std::string sender, std::string log);
  void SendPacket(std::string sender, std::string type, std::string messsage);

  void WriteImageListToFile(const std::string fn,
                            const std::vector< std::string > &imlist);
  void CreateFileList(boost::filesystem::path dir);

  void InitEngineResources( std::string name );
  void RunClusterCommand();
};

#endif /* _VISE_SEARCH_ENGINE_H */
