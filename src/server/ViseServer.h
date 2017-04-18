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

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "SearchEngine.h"
#include "ViseMessageQueue.h"

using boost::asio::ip::tcp;

// defined in src/vise.cc
// a global message queue to send communications to client HTTP browser
extern ViseMessageQueue vise_message_queue_;

class ViseServer {
public:
  ViseServer();

  void InitResources( std::string vise_datadir, std::string html_template_dir );
  void Start(unsigned int port);
  bool Stop();
  bool Restart();

  std::string GetViseDataDir() {
    return vise_datadir_.string();
  }
  std::string GetHtmlTemplateDir() {
    return html_template_dir_.string();
  }

private:
  SearchEngine     search_engine_;

  unsigned int port_;
  std::string hostname_;
  std::string url_prefix_;
  std::string msg_url_;

  boost::filesystem::path vise_datadir_;
  boost::filesystem::path html_template_dir_;

  // location of search engine temporary files
  boost::filesystem::path vise_enginedir_;
  boost::filesystem::path vise_htmldir_;
  boost::filesystem::path vise_main_html_fn_;

  // html contents
  std::string html_vise_main_;
  std::vector<std::string> state_html_list_;

  boost::system::error_code error_;

  void Read();
  void Evaluate();
  void Print();
  void CloseConnection();
  void MoveToNextState();

  void HandleConnection(boost::shared_ptr<tcp::socket> p_socket);
  void HandleGetRequest(std::string resource, boost::shared_ptr<tcp::socket> p_socket);

  void HandlePostRequest(std::string search_engine_name,
                         std::string resource,
                         std::string post_data,
                         boost::shared_ptr<tcp::socket> p_socket);

  void InitializeSearchEngine(std::string search_engine_name);

  void SendHttpResponse(std::string html, boost::shared_ptr<tcp::socket> p_socket);
  void SendHttpNotFound(boost::shared_ptr<tcp::socket> p_socket);
  void SendHttpRedirect(std::string redirect_uri, boost::shared_ptr<tcp::socket> p_socket);
  void SendErrorResponse(std::string message, std::string backtrace, boost::shared_ptr<tcp::socket> p_socket);
  void SendRawResponse(std::string response, boost::shared_ptr<tcp::socket> p_socket);
  void SendJsonResponse(std::string json, boost::shared_ptr<tcp::socket> p_socket);

  void SendMessage(std::string sender, std::string message);
  void SendMessage(std::string message);
  void SendStatus(std::string sender, std::string status);
  void SendStatus(std::string status);

  void ExtractHttpResource(std::string http_request, std::string &http_resource);
  void ExtractHttpContent(std::string http_request, std::string &http_content);

  int  LoadStateHtml(unsigned int state_id, std::string &state_html);
  int  LoadFile(std::string filename, std::string &file_contents);
  void WriteFile(std::string filename, std::string &file_contents);

  void SplitString(std::string s, char sep, std::vector<std::string> &tokens);
  bool ReplaceString(std::string &s, std::string old_str, std::string new_str);
};

#endif /* _VISE_SERVER_H */
