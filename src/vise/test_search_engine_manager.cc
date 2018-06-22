/** @file   test_search_engine_manager.cc
 *  @brief  tests for search_engine_manager.cc
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   20 June 2018
 */

#include <iostream>
#include <string>
#include <map>

#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>

#include "vise/search_engine_manager.h"
#include "http_server/http_response.h"

using namespace std;

bool test_search_engine_manager_add_image() {
  http_response response;
  map<string, string> uri_param;
  string search_engine_name = "ox5k";
  string search_engine_version = "1";
  string search_engine_command = "add_file?filename=\"all_souls_00001.jpg\"";
  string payload = "base64 image data";
  search_engine_manager::instance()->process_cmd(search_engine_name, search_engine_version, search_engine_command, uri_param, payload, response);
}

int main(int argc, char** argv) {
  boost::filesystem::path data_dir( boost::filesystem::temp_directory_path() / "vise" );
  boost::filesystem::path search_engine_data_dir = data_dir / "repo";
  search_engine_manager::instance()->init(search_engine_data_dir);

  test_search_engine_manager_add_image();
}
