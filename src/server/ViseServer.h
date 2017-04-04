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
#include <string>
#include <sstream>

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
  std::string  hostname_;
  boost::filesystem::path  vise_datadir_;
  boost::filesystem::path  vise_enginedir_;
  boost::filesystem::path  vise_htmldir_;
  SearchEngine search_engine_;

  boost::system::error_code error_;

  void HandleConnection(boost::asio::ip::tcp::socket socket);
  void Read();
  void Evaluate();
  void Print();
  void CloseConnection();
  void MoveToNextState();

  void SplitString(std::string s, char sep, std::vector<std::string> &tokens);
};

#endif /* _VISE_SERVER_H */
