/** @file   vise_request_handler.h
 *  @brief  A singleton class which handles http requests related to vise
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   18 June 2018
 */

#ifndef _VISE_REQUEST_HANDLER_H_
#define _VISE_REQUEST_HANDLER_H_

#include <iostream>
#include <sstream>
#include <fstream>

#include <boost/filesystem.hpp>

// to generate uuid
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <Magick++.h>            // to transform images

#include <eigen3/Eigen/Dense>

#include "http_server/http_request.h"
#include "http_server/http_response.h"

using namespace std;
using namespace Eigen;

// uses C++ singleton design pattern
class vise_request_handler {
  boost::filesystem::path upload_dir_;
  boost::filesystem::path result_dir_;
  boost::filesystem::path asset_dir_;

  vise_request_handler() { };
  vise_request_handler(const vise_request_handler& sh) { };
  vise_request_handler* operator=(const vise_request_handler &) {
    return 0;
  }

  static vise_request_handler* vise_request_handler_;

  // _upload
  bool save_user_upload(const http_request& request, string& fid);

  // _compare
  void register_images();

  // result
  bool has_invalid_char(const std::string s);
  bool load_file_contents(const boost::filesystem::path fn,
                          std::string& file_contents);

  public:
  static vise_request_handler* instance();

  void init(const boost::filesystem::path upload_dir,
            const boost::filesystem::path result_dir,
            const boost::filesystem::path asset_dir);
  void handle_http_request(const http_request& request, http_response& response);
};
#endif
