/** @file   vise.cc
 *  @brief  main entry point for the VGG Image Search Enginer (VISE)
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   31 March 2017
 */

#include <iostream>
#include <string>

#include <boost/filesystem.hpp>

#include <Magick++.h>            // to transform images

#include "ViseServer.h"

int main(int argc, char** argv) {

  std::cout << "VGG Image Search Engine (VISE)";
  std::cout << "\nversion 1.0.2\n";
  //std::cout << "\nAuthor: Abhishek Dutta <adutta@robots.ox.ac.uk>, May 2017\n";
  //std::cout << "\nVISE builds on the \"relja_retrival\" (Sep. 2014) C++ codebase \nauthored by Relja Arandjelovic <relja@robots.ox.ac.uk> during \nhis DPhil / Postdoc at the Visual Geometry Group in the \nDepartment of Engineering Science, University of Oxford." << std::endl;

  if ( argc != 4 ) {
    std::cout << "\n  Usage: ./vise VISE_SOURCE_CODE_DIR VISE_APPLICATION_DATA_DIR VISE_TRAINING_IMAGES_DIR\n" << std::flush;
    return 0;
  }

  unsigned int port = 9971;
  //unsigned int port = 8080;

  Magick::InitializeMagick(*argv);

/*
  std::string vise_src = getenv("VISE_SRC_DIR");
  std::string data_home = getenv("VISE_DATA_DIR");
*/

  boost::filesystem::path vise_src_code_dir( argv[1] );
  boost::filesystem::path vise_application_data_dir( argv[2] );
  boost::filesystem::path vise_training_images_dir( argv[3] );

  if (!boost::filesystem::exists(vise_src_code_dir) ) {
    std::cout << "\nVISE_SOURCE_CODE_DIR = " << vise_src_code_dir.string() << " does not exist!" << std::flush;
    std::cout << std::endl;
    return 0;
  }

  if ( !boost::filesystem::exists(vise_application_data_dir) ) {
    boost::filesystem::create_directories( vise_application_data_dir );
    std::cout << "\nCreated VISE_APPLICATION_DATA_DIR=" << vise_application_data_dir.string() << std::endl;
  }

  //std::cout << "\nVISE_SRC_CODE_DIR = " << vise_src_code_dir.string();
  //std::cout << "\nVISE_APPLICATION_DATA_DIR = " << vise_application_data_dir.string() << std::flush;
  //std::cout << "\nVISE_TRAINING_IMAGES_DIR = " << vise_training_images_dir.string() << std::flush;

  ViseServer vise_server( vise_application_data_dir, vise_training_images_dir, vise_src_code_dir );
  //vise_server.InitResources( visedata_dir, template_dir );

  vise_server.Start(port);

  // server is stopped by sending the following HTTP POST request
  // POST /
  // shutdown_vise now (the POST data)
  return 0;
}
