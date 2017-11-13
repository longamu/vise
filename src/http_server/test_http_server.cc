#include <iostream>
#include <http_server.h>

#include <boost/filesystem.hpp>

using namespace std;

int main(int argc, char** argv) {
  cout << "unit test for http server" << endl;

  std::string address = "0.0.0.0";
  unsigned int port = 9973;
  size_t thread_pool_size = 4;
  
  boost::filesystem::path app_basedir = boost::filesystem::temp_directory_path() / "http_server";
  http_server s;

  return 0;
}
