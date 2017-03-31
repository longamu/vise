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

using boost::asio::ip::tcp;

class ViseServer {
 public:
  ViseServer();

  void Start(unsigned int port);
  bool Stop();
  bool Restart();

  void HandleNewConnection();
  void Read();
  void Evaluate();
  void Print();
  void CloseConnection();

 private:
  unsigned int port_;
  std::string  hostname_;
  std::string  data_basedir_;

  boost::system::error_code error_;
};

#endif /* _VISE_SERVER_H */
