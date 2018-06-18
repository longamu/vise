/** @file   vise_server.cc
 *  @brief  main entry point for the vise server
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   18 June 2018
 */

#include <cstdio>
#include <csignal>

#include <iostream>
#include <string>
#include <thread>

#include <boost/filesystem.hpp>
#include <Magick++.h>            // to transform images

#include "vise_server_config.h"
#include "http_server/http_server.h"
#include "vise/vise_request_handler.h"

#if defined(_WIN32) || defined(WIN32)
  #include <windows.h>
  #include <ShellAPI.h>
#endif

int main(int argc, char** argv) {  
  std::cout << VISE_SERVER_NAME << " "
            << VISE_SERVER_VERSION_MAJOR << "."
            << VISE_SERVER_VERSION_MINOR << "."
            << VISE_SERVER_VERSION_PATCH
            << " [" << VISE_SERVER_URL << "]";

  std::cout << "\nAuthor: "
            << VISE_SERVER_AUTHOR_NAME << " <"
            << VISE_SERVER_AUTHOR_EMAIL << ">"
//            << "\nRelease: " << VISE_SERVER_CURRENT_RELEASE_DATE
            << std::endl;

  if ( argc != 6 && argc != 7 && argc != 1) {
    std::cout << "\nUsage: " << argv[0] << " hostname port thread_count asset_dir [application_data_dir | [upload_dir result_dir] ]\n" << std::flush;
    return 0;
  }
  
  // default values
  std::string address("0.0.0.0");
  std::string port("9973");
  boost::filesystem::path exec_dir( argv[0] );
  
  Magick::InitializeMagick(exec_dir.parent_path().string().c_str());
  std::cout << "\nMagick::InitializeMagick = " << exec_dir.parent_path().string().c_str() << std::endl;
  
  boost::filesystem::path asset_dir( exec_dir.parent_path() / "asset");
  unsigned int thread_pool_size = 3;
  
  boost::filesystem::path temp_dir( boost::filesystem::temp_directory_path() / "vise" );
  boost::filesystem::path upload_dir = temp_dir / "upload";
  boost::filesystem::path result_dir = temp_dir / "result";
  
  if( argc == 6 || argc == 7 ) {
    address = argv[1];
    port    = argv[2];
    asset_dir = boost::filesystem::path(argv[4]);

    std::stringstream s;
    s.clear(); s.str(argv[3]);
    s >> thread_pool_size;

    if ( argc == 6 ) {
      boost::filesystem::path app_dir( argv[5] );
      upload_dir = app_dir / "upload";
      result_dir = app_dir / "result";
    }
    if ( argc == 7 ) {
      upload_dir = boost::filesystem::path(argv[5]);
      result_dir = boost::filesystem::path(argv[6]);
    }
  } else {
    cout << "\nUsing default settings ...";
  }
  
  if ( !boost::filesystem::exists(upload_dir) ) {
    boost::filesystem::create_directories(upload_dir);
  } else {
    // cleanup
    boost::filesystem::remove_all( upload_dir );
    boost::filesystem::create_directories(upload_dir);
  }
  if ( !boost::filesystem::exists(result_dir) ) {
    boost::filesystem::create_directories(result_dir);
  } else {
    // cleanup
    boost::filesystem::remove_all( result_dir );
    boost::filesystem::create_directories(result_dir);
  }
  
  vise_request_handler::instance()->init(upload_dir, result_dir, asset_dir);

  http_server server(address, port, thread_pool_size);
  std::cout << "\n\nNotes:";
  std::cout << "\n  - To use the application, visit http://localhost:" << port << "/vise/ in a web browser" << flush;

  server.start();
  return 0;
}
