/** @file   vise_server.h
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
#include <ctime>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "SearchEngine.h"

using boost::asio::ip::tcp;

class ViseServer {
 public:
  ViseServer( std::string vise_datadir );

  void Start(unsigned int port);
  bool Stop();
  bool Restart();

 private:
  unsigned int port_;
  std::string hostname_;
  std::string url_prefix_;

  boost::filesystem::path  vise_datadir_;
  boost::filesystem::path  vise_enginedir_;
  boost::filesystem::path  vise_htmldir_;
  SearchEngine search_engine_;

  // html templates
  std::string html_engine_settings_;
  std::string html_engine_training_;

  boost::system::error_code error_;

  void HandleConnection(boost::asio::ip::tcp::socket socket);
  void Read();
  void Evaluate();
  void Print();
  void CloseConnection();
  void MoveToNextState();

  void HttpRedirect( std::string redirect_url, boost::asio::ip::tcp::iostream &httpstream);
  void HandleRequest(std::string resource, boost::asio::ip::tcp::iostream &httpstream);
  void SendHtml(std::string html, boost::asio::ip::tcp::iostream &httpstream);
  void HttpError(std::string error_code, boost::asio::ip::tcp::iostream &httpstream);

  void LoadFile(std::string filename, std::string &file_contents);
  void WriteFile(std::string filename, std::string &file_contents);


  void SplitString(std::string s, char sep, std::vector<std::string> &tokens);
  bool ReplaceString(std::string &s, std::string old_str, std::string new_str);
};

#endif /* _VISE_SERVER_H */
