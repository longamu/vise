/** @file   vise.cc
 *  @brief  main entry point for the VGG Image Search Enginer (VISE)
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   31 March 2017
 */

#include <iostream>
#include <string>

#include <Magick++.h>            // to transform images

#include "ViseServer.h"

// global context ensure that any object can send messages to the client
ViseMessageQueue vise_message_queue_;

int main(int argc, char** argv) {
  std::cout << "\nVGG Image Search Engine (VISE)";
  std::cout << "\n";
  unsigned int port = 8080;

  Magick::InitializeMagick(*argv);

  std::string visedata_dir = "/home/tlm/vise/";
  std::string html_template_dir = "/home/tlm/dev/vise/src/server/html_templates/";
  ViseServer vise_server;
  vise_server.InitResources( visedata_dir, html_template_dir );

  vise_server.Start(port);

  std::string user_input;
  while ( user_input != "q" ) {
    std::cout << "\nPress \"q\" + [Enter] key to stop the server : ";
    std::cin >> user_input;
  }

  if ( vise_server.Stop() ) {
    std::cout << "\nBye\n";
    return 0;
  } else {
    std::cerr << "\nFailed to stop server!\n";
    return -1;
  }
}
