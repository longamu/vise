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
  MPI_INIT_ENV
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

  if ( argc != 5 && argc != 6 && argc != 1) {
    std::cout << "\nUsage: " << argv[0]
              << " hostname port thread_count asset_dir [application_data_dir]\n"
              << std::flush;
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

  boost::filesystem::path data_dir( boost::filesystem::temp_directory_path() / "vise" );

  if( argc == 5 || argc == 6 ) {
    address = argv[1];
    port    = argv[2];
    asset_dir = boost::filesystem::path(argv[4]);

    std::stringstream s;
    s.clear(); s.str(argv[3]);
    s >> thread_pool_size;

    if ( argc == 6 ) {
      data_dir = boost::filesystem::path( argv[5] );
    }
  } else {
    cout << "\nUsing default settings ...";
  }

  if ( !boost::filesystem::exists(data_dir) ) {
    boost::filesystem::create_directories(data_dir);
  } else {
    // cleanup
    //boost::filesystem::remove_all( data_dir );
    //boost::filesystem::create_directories(data_dir);
  }

  boost::filesystem::path search_engine_data_dir = data_dir / "repo";
  if ( !boost::filesystem::exists(search_engine_data_dir) ) {
    boost::filesystem::create_directories( search_engine_data_dir );
  } else {
    // cleanup
    //boost::filesystem::remove_all( search_engine_data_dir );
    //boost::filesystem::create_directories( search_engine_data_dir );
  }

  // this is critical to avoid race conditions for vise_request_handler::instance()
  // initialize http request handler
  vise_request_handler::instance()->init(data_dir, asset_dir);
  // initialize search engine manager
  search_engine_manager::instance()->init(search_engine_data_dir);

  http_server server(address, port, thread_pool_size);
  std::cout << "\n\nNotes:";
  std::cout << "\n  - To use the application, visit http://localhost:" << port << "/vise/ in a web browser" << std::endl;

  server.start();
  return 0;
}
