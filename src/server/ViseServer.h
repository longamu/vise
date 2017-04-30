/** @file   ViseServer.h
 *  @brief  provides HTTP based user interface for VISE
 *
 *  Provides a HTTP based user interface to configure, train and query the
 *  VGG Image Search Enginer (VISE)
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   31 March 2017
 */

#ifndef _VISE_SERVER_H
#define _VISE_SERVER_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/timer/timer.hpp>

#include "SearchEngine.h"
#include "ViseMessageQueue.h"

using boost::asio::ip::tcp;

// defined in src/vise.cc
// a global message queue to send communications to client HTTP browser
extern ViseMessageQueue vise_message_queue_;

class ViseServer {
 private:
  SearchEngine search_engine_;
  unsigned int port_;
  std::string hostname_;
  std::string url_prefix_;
  std::string msg_url_;

  int state_id_;
  std::vector< int > state_id_list_;
  std::vector< std::string > state_name_list_;
  std::vector< std::string > state_info_list_;
  std::vector< std::string > state_html_fn_list_;
  std::vector< std::string > state_html_list_;

  // html content
  std::string vise_main_html_;
  std::string vise_help_html_;
  std::string vise_404_html_;
  std::string vise_css_;
  std::string vise_js_;

  std::string vise_index_html_;
  bool vise_index_html_reload_;

  // html content location
  boost::filesystem::path vise_css_fn_;
  boost::filesystem::path vise_js_fn_;
  boost::filesystem::path vise_main_html_fn_;
  boost::filesystem::path vise_help_html_fn_;
  boost::filesystem::path vise_404_html_fn_;

  // resource dir
  boost::filesystem::path vise_datadir_;
  boost::filesystem::path vise_templatedir_;
  boost::filesystem::path vise_logdir_;
  boost::filesystem::path vise_enginedir_;

  boost::system::error_code error_;

  // state maintainanace
  bool UpdateState();
  std::string GetStateJsonData();
  void GenerateViseIndexHtml();
  void HandleStateGetRequest( int state_id,
                              std::map< std::string, std::string > args,
                              boost::shared_ptr<tcp::socket> p_socket );
  void ServeStaticResource(std::map< std::string, std::string > resource_args,
                            boost::shared_ptr<tcp::socket> p_socket);
  void HandleStatePostData( int state_id, std::string http_post_data, boost::shared_ptr<tcp::socket> p_socket );
  void SetSearchEngineSetting( std::string setting );

  // search engine training
  void InitiateSearchEngineTraining();

  // HTTP connection handler
  void HandleConnection(boost::shared_ptr<tcp::socket> p_socket);
  void HandleGetRequest(std::string resource, boost::shared_ptr<tcp::socket> p_socket);
  void HandlePostRequest(std::string search_engine_name,
                         std::string resource,
                         std::string post_data,
                         boost::shared_ptr<tcp::socket> p_socket);
  void SendHttpResponse(std::string html, boost::shared_ptr<tcp::socket> p_socket);
  void SendHttpPostResponse(std::string http_post_data, std::string result, boost::shared_ptr<tcp::socket> p_socket);
  void SendHttp404NotFound(boost::shared_ptr<tcp::socket> p_socket);
  void SendHttpRedirect(std::string redirect_uri, boost::shared_ptr<tcp::socket> p_socket);
  void SendErrorResponse(std::string message, std::string backtrace, boost::shared_ptr<tcp::socket> p_socket);
  void SendRawResponse(std::string content_type, std::string content, boost::shared_ptr<tcp::socket> p_socket);
  void SendJsonResponse(std::string json, boost::shared_ptr<tcp::socket> p_socket);

  void SendMessage(std::string message);
  void SendLog(std::string log);
  void SendCommand(std::string command);
  void SendPacket(std::string type, std::string message);

  // helper functions
  void ExtractHttpResource(std::string http_request, std::string &http_resource);
  void ExtractHttpContent(std::string http_request, std::string &http_content);
  void ParseHttpMethodUri(const std::string http_method_uri,
                          std::string &resource_name,
                          std::map< std::string, std::string > &resource_args);
  int  LoadFile(std::string filename, std::string &file_contents);
  void WriteFile(std::string filename, std::string &file_contents);
  void SplitString(const std::string s, char sep, std::vector<std::string> &tokens);
  bool ReplaceString(std::string &s, std::string old_str, std::string new_str);

  // for logging statistics
  boost::filesystem::path vise_training_stat_fn_;

  std::ofstream training_stat_f;
  void AddTrainingStat(std::string dataset_name, std::string state_name, unsigned long time_sec, unsigned long space_bytes);

 public:
  static const int STATE_NOT_LOADED =  0;
  static const int STATE_SETTING    =  1;
  static const int STATE_INFO       =  2;
  static const int STATE_PREPROCESS =  3;
  static const int STATE_DESCRIPTOR =  4;
  static const int STATE_CLUSTER    =  5;
  static const int STATE_ASSIGN     =  6;
  static const int STATE_HAMM       =  7;
  static const int STATE_INDEX      =  8;
  static const int STATE_QUERY      =  9;

  ViseServer( std::string vise_datadir, std::string vise_templatedir );
  ~ViseServer();

  std::string GetCurrentStateName();
  std::string GetCurrentStateInfo();
  std::string GetStateName( int state_id );
  std::string GetStateInfo( int state_id );
  int GetStateId( std::string );

  //void InitResources( std::string vise_datadir, std::string vise_templatedir );
  void Start(unsigned int port);
  bool Stop();
  bool Restart();
};

#endif /* _VISE_SERVER_H */
