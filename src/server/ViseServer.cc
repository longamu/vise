#include "ViseServer.h"

ViseServer::ViseServer(std::string vise_datadir) {
  vise_datadir_   = boost::filesystem::path(vise_datadir);
  vise_enginedir_ = vise_datadir_ / "search_engines";
  vise_htmldir_   = vise_datadir_ / "html";

  if ( !is_directory(vise_datadir_) ) {
    create_directory(vise_datadir_);
  }

  if ( !is_directory(vise_enginedir_) ) {
    create_directory(vise_enginedir_);
  }

  if ( !is_directory(vise_htmldir_) ) {
    create_directory(vise_htmldir_);
  }

  // @todo
  html_dir_ = boost::filesystem::path("/home/tlm/dev/vise/src/server/html_templates/");
  vise_main_html_fn_ = html_dir_ / "vise_main.html";
  LoadFile(vise_main_html_fn_.string(), html_vise_main_);
}

void ViseServer::Start(unsigned int port) {
  hostname_ = "localhost";
  port_ = port;

  std::ostringstream port_str;
  port_str << port_;
  url_prefix_ = "http://" + hostname_ + ":" + port_str.str();

  boost::asio::io_service io_service;
  boost::asio::ip::tcp::endpoint endpoint( tcp::v4(), port_ );
  tcp::acceptor acceptor ( io_service , endpoint );
  acceptor.set_option( tcp::acceptor::reuse_address(true) );

  std::cout << "\nServer started on port " << port << " :-)";

  // for debugging, we initial a search engine
  std::string search_engine_name = "ox5k";
  search_engine_.Init(search_engine_name, vise_enginedir_);

  std::string search_engine_base_url = url_prefix_;
  search_engine_base_url += search_engine_.GetSearchEngineBaseUri();
  ReplaceString(html_vise_main_,
                "__VISE_SERVER_ADDRESS__",
                search_engine_base_url);

  // load html file resources
  for (unsigned int i=0; i < SearchEngine::STATE_COUNT; i++) {
    state_html_list_.push_back("");
    LoadStateHtml(i, state_html_list_.at(i));
  }

  try {
    while ( 1 ) {
      boost::shared_ptr<tcp::socket> p_socket( new tcp::socket(io_service) );
      acceptor.accept( *p_socket );
      boost::thread t( boost::bind( &ViseServer::HandleConnection, this, p_socket ) );
    }
  } catch (std::exception &e) {
    std::cerr << "\nCannot listen for http request!\n" << e.what() << std::flush;
    return;
  }
}

bool ViseServer::Stop() {
  std::cout << "\nServer stopped!";
  return true;
}

void ViseServer::SplitString( std::string s, char sep, std::vector<std::string> &tokens ) {
  unsigned int start = 0;
  for ( unsigned int i=0; i<s.length(); ++i) {
    if ( s.at(i) == sep ) {
      tokens.push_back( s.substr(start, (i-start)) );
      start = i + 1;
    }
  }
  tokens.push_back( s.substr(start, s.length()) );
}

void ViseServer::HandleConnection(boost::shared_ptr<tcp::socket> p_socket) {
  char http_buffer[1024];

  // get the http_method : {GET, POST, ...}
  size_t len = p_socket->read_some(boost::asio::buffer(http_buffer), error_);
  if ( error_ ) {
    std::cerr << "\nViseServer::HandleConnection() : error using read_some()\n"
              << error_.message() << std::flush;
  }
  std::string http_method  = std::string(http_buffer, 4);
  std::string http_request = std::string(http_buffer, len);
  std::string http_method_uri;
  ExtractHttpResource(http_request, http_method_uri);

  // for debug
  //std::cout << "\nRequest = " << http_request << std::flush;
  std::cout << "\nMethod = [" << http_method << "]" << std::flush;
  std::cout << "\nResource = [" << http_method_uri << "]" << std::flush;

  std::vector<std::string> tokens;
  SplitString( http_method_uri, '/', tokens );

  // debug
  for ( unsigned int i=0; i<tokens.size(); i++ ) {
    std::cout << "\ntokens[" << i << "] = " << tokens.at(i) << std::flush;
  }

  std::string search_engine_name;
  if ( http_method == "GET " ) {
    if ( tokens.at(0) == "" && tokens.at(1).length() != 0 ) {
      if ( tokens.size() < 3 ) {
        if ( tokens.at(1) == "favicon.ico" ) {
          // @todo: implement this in the future
          SendHttpNotFound( p_socket );
        } else {
          // http://localhost:8080/search_engine_name
          search_engine_name = tokens.at(1);
          if ( search_engine_name == search_engine_.Name() ) {
            SendHttpResponse(html_vise_main_, p_socket);
          } else {
            SendHttpNotFound( p_socket );
          }
        }
      } else {
        std::string resource = tokens.at(2);
        HandleGetRequest(resource, p_socket);
      }
    } else {
      SendHttpNotFound( p_socket );
    }
  } else if ( http_method == "POST" ) {
    if ( tokens.at(0) == "" && tokens.at(1).length() != 0 ) {
      if ( tokens.size() < 3 ) {
        SendHttpNotFound( p_socket );
      } else {
        // http://localhost:8080/search_engine_name
        search_engine_name = tokens.at(1);
        if ( search_engine_name == search_engine_.Name() ) {
          std::string resource = tokens.at(2);
          std::string http_post_data;
          ExtractHttpContent(http_request, http_post_data);

          HandlePostRequest( resource, http_post_data, p_socket);
        } else {
          SendHttpNotFound( p_socket );
        }
      }
    }
  } else {
    std::cerr << "\nUnknown http_method : " << http_method << std::flush;
    SendHttpNotFound( p_socket );
  }

  /*
  if ( http_method == "GET " ) {
    if ( search_engine_.GetEngineState() == SearchEngine::UNKNOWN ) {
      if ( tokens.at(0) == "" && tokens.at(1).length() != 0 ) {
        // http://localhost:8080/search_engine_name
        std::string search_engine_name = tokens.at(1);
        search_engine_.Init(search_engine_name, vise_enginedir_);

        std::string redirect_uri = search_engine_.MoveToNextState();
        SendHttpRedirect(redirect_uri, p_socket);
      } else {
        SendHttpNotFound( p_socket );
      }
    } else {
      if ( tokens.at(0) == "" && tokens.at(1) == "favicon.ico" ) {
        SendHttpNotFound( p_socket );
      } else {
        HandleRequest(tokens.at(2), p_socket);
      }
    }
  } else if ( http_method == "POST" ) {
    // read
    std::string http_post_data;
    ExtractHttpContent(http_request, http_post_data);

    // evaluate
    if ( tokens.at(2) == "settings" ) {
      search_engine_.SetEngineConfig(http_post_data);

      std::string engine_config_path = search_engine_.GetEngineConfigPath().string();
      WriteFile( engine_config_path, http_post_data );

      // reply (or redirect)
      std::string redirect_uri = search_engine_.MoveToNextState();
      SendHttpRedirect(redirect_uri, p_socket);

    } else if ( tokens.at(2) == "training" ) {
      std::cout << "\nRunning training command : " << http_post_data << std::flush;
      if ( http_post_data.at(0) == '$' ) {
        // execute the command
        http_post_data = http_post_data.substr(1); // remove $
        std::vector< std::string> cmd_params;
        SplitString(http_post_data, ' ', cmd_params);
        std::string cmd = cmd_params.at(0);
        cmd_params.erase( cmd_params.begin() );
        SendRawResponse(http_post_data, p_socket);

      } else if ( http_post_data.at(0) == '?' ) {
        // report status of command execution
        // @todo
      } else {
        SendErrorResponse(http_method + http_method_uri,
                          http_request,
                          p_socket);
      }
    } else {
      SendErrorResponse(http_method + http_method_uri,
                        http_request,
                        p_socket);
    }
  } else {
    SendHttpNotFound( p_socket );
  }
  */
  p_socket->close();
}


void ViseServer::HandlePostRequest( std::string resource_name,
                                    std::string post_data,
                                    boost::shared_ptr<tcp::socket> p_socket)
{
  std::cout << "\nReceived post data : resource=" << resource_name << ", post_data=" << post_data << std::flush;
}

void ViseServer::HandleGetRequest( std::string resource_name,
                                   boost::shared_ptr<tcp::socket> p_socket)
{
  std::cout << "\nHandleRequest() : " << resource_name << std::flush;

  int state_id = search_engine_.GetEngineState(resource_name);
  std::cout << "\nstate_id = " << state_id << std::flush;
  if ( state_id >= 0 ) {
    // request for engine state html
    if ( state_html_list_.at(state_id).length() != 0 ) {
      std::cout << "\nServing state html page : " << resource_name << std::flush;
      SendHttpResponse( state_html_list_.at(state_id), p_socket );
    }
  } else if ( resource_name == "state_list" ) {
    SendJsonResponse( search_engine_.GetEngineStateList(), p_socket );
  } else {
    SendHttpNotFound(p_socket);
  }

  /*
    if ( resource_name == "settings" ) {
    // send the configure html
    if ( html_engine_settings_.length()  == 0 ) {
    LoadFile("/home/tlm/dev/vise/src/server/html_templates/settings.html", html_engine_settings_);
    std::string post_url = url_prefix_;
    post_url += search_engine_.GetResourceUri("settings");
    ReplaceString(html_engine_settings_,
    "__VISE_SEARCH_ENGINE_SETTINGS_POST_URL__",
    post_url);

    std::string get_url = url_prefix_;
    get_url += search_engine_.GetResourceUri("training");
    ReplaceString(html_engine_settings_,
    "__VISE_SEARCH_ENGINE_TRAINING_GET_URL__",
    get_url);

    }
    SendHttpResponse( html_engine_settings_, p_socket );
    } else if ( resource_name == "training" ) {
    // show the training status and progress page
    std::cout << "\nhtml_engine_training_.length() = " << html_engine_training_.length() << std::flush;
    if ( html_engine_training_.length()  == 0 ) {
    LoadFile("/home/tlm/dev/vise/src/server/html_templates/training.html", html_engine_training_);

    std::string post_url = url_prefix_;
    post_url += search_engine_.GetResourceUri("training");
    std::cout << "\npost_url = " << post_url << std::flush;
    ReplaceString(html_engine_training_,
    "__VISE_SEARCH_ENGINE_TRAINING_POST_URL__",
    post_url);
    }
    SendHttpResponse( html_engine_training_, p_socket );
    } else if ( resource_name == "state_list" ) {
    std::cout << "\nGetting state_list" << std::flush;
    SendRawResponse( search_engine_.GetEngineStateList(), p_socket );
    } else {
    SendHttpNotFound(p_socket);
    }
  */
}

void ViseServer::SendErrorResponse(std::string message, std::string backtrace, boost::shared_ptr<tcp::socket> p_socket) {
  std::string html;
  html  = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">";
  html += "<title>VGG Image Search Engine</title></head>";
  html += "<body><h1>Error !</h1>";
  html += "<p>" + message + "</p>";
  html += "<p><pre>" + backtrace + "</pre></p>";
  std::time_t t = std::time(NULL);
  char date_str[100];
  std::strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %Z", std::gmtime(&t));
  html += "<p>&nbsp;</p>";
  html += "<p>Generated on " + std::string(date_str) + "</p>";
  html += "</body></html>";

  SendHttpResponse(html, p_socket);
}

void ViseServer::SendRawResponse(std::string response, boost::shared_ptr<tcp::socket> p_socket) {
  std::stringstream http_response;
  http_response << "HTTP/1.1 200 OK\r\n";
  http_response << "Content-type: text/html; charset=utf-8\r\n";
  http_response << "Content-Length: " << response.length() << "\r\n";
  http_response << "\r\n";
  http_response << response;

  boost::asio::write( *p_socket, boost::asio::buffer(http_response.str()) );
  std::cout << "\nSent response : [" << response << "]" << std::flush;
}

void ViseServer::SendJsonResponse(std::string json, boost::shared_ptr<tcp::socket> p_socket) {
  std::stringstream http_response;
  http_response << "HTTP/1.1 200 OK\r\n";
  http_response << "Content-type: application/json; charset=utf-8\r\n";
  http_response << "Content-Length: " << json.length() << "\r\n";
  http_response << "\r\n";
  http_response << json;

  boost::asio::write( *p_socket, boost::asio::buffer(http_response.str()) );
  std::cout << "\nSent json response : [" << json << "]" << std::flush;
}

void ViseServer::SendHttpResponse(std::string response, boost::shared_ptr<tcp::socket> p_socket) {
  std::stringstream http_response;
  http_response << "HTTP/1.1 200 OK\r\n";
  std::time_t t = std::time(NULL);
  char date_str[100];
  std::strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %Z", std::gmtime(&t));
  http_response << "Date: " << date_str << "\r\n";
  http_response << "Content-Language: en\r\n";
  http_response << "Connection: close\r\n";
  http_response << "Cache-Control: no-cache\r\n";
  http_response << "Content-type: text/html; charset=utf-8\r\n";
  http_response << "Content-Encoding: utf-8\r\n";
  http_response << "Content-Length: " << response.length() << "\r\n";
  http_response << "\r\n";
  http_response << response;
  boost::asio::write( *p_socket, boost::asio::buffer(http_response.str()) );

  std::cout << "\nSent http html response of length : " << response.length() << std::flush;
}

void ViseServer::SendHttpNotFound(boost::shared_ptr<tcp::socket> p_socket) {
  std::string html;
  html  = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">";
  html += "<title>VGG Image Search Engine</title></head>";
  html += "<body><h1>You seem to be lost.</h1>";
  html += "<p>Don't despair. Getting lost is a good sign -- it means that you are exploring.</p>";
  html += "</body></html>";

  std::stringstream http_response;

  http_response << "HTTP/1.1 404 Not Found\r\n";
  std::time_t t = std::time(NULL);
  char date_str[100];
  std::strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %Z", std::gmtime(&t));
  http_response << "Date: " << date_str << "\r\n";
  http_response << "Content-type: text/html\r\n";
  http_response << "Content-Length: " << html.length() << "\r\n";
  http_response << "\r\n";
  http_response << html;
  boost::asio::write( *p_socket, boost::asio::buffer(http_response.str()) );
}

void ViseServer::SendHttpRedirect( std::string redirect_uri,
                                   boost::shared_ptr<tcp::socket> p_socket)
{
  std::stringstream http_response;
  http_response << "HTTP/1.1 303 See Other\r\n";
  http_response << "Location: " << (url_prefix_ + redirect_uri) << "\r\n";
  std::cout << "\nRedirecting to : " << (url_prefix_ + redirect_uri) << std::flush;
  boost::asio::write( *p_socket, boost::asio::buffer(http_response.str()) );
}

void ViseServer::ExtractHttpResource(std::string http_request, std::string &http_resource) {
  unsigned int first_space  = http_request.find(' ', 0);
  unsigned int second_space = http_request.find(' ', first_space+1);

  unsigned int length = (second_space - first_space);
  http_resource = http_request.substr(first_space+1, length-1);
}

void ViseServer::ExtractHttpContent(std::string http_request, std::string &http_content) {
  std::string http_content_start_flag = "\r\n\r\n";
  unsigned int start = http_request.rfind(http_content_start_flag);
  http_content = http_request.substr( start + http_content_start_flag.length() );
}

int ViseServer::LoadFile(std::string filename, std::string &file_contents) {
  try {
    std::ifstream f;
    f.open(filename.c_str());
    f.seekg(0, std::ios::end);
    file_contents.reserve( f.tellg() );
    f.seekg(0, std::ios::beg);

    file_contents.assign( std::istreambuf_iterator<char>(f),
                          std::istreambuf_iterator<char>() );
    f.close();
    return 1;
  } catch (std::exception &e) {
    std::cerr << "\nViseServer::LoadFile() : failed to load file : " << filename << std::flush;
    file_contents = "";
    return 0;
  }
}

 int ViseServer::LoadStateHtml(unsigned int state_id, std::string &state_html) {
  std::string state_name = search_engine_.GetEngineStateName(state_id);
  boost::filesystem::path htmlfn = html_dir_ / boost::filesystem::path(state_name + ".html");
  if ( LoadFile(htmlfn.string(), state_html) ) {
    return 1;
  } else {
    return 0;
  }
}

void ViseServer::WriteFile(std::string filename, std::string &file_contents) {
  try {
    std::ofstream f;
    f.open(filename.c_str());
    f << file_contents;
    f.close();
  } catch (std::exception &e) {
    std::cerr << "\nViseServer::WriteFile() : failed to save file : " << filename << std::flush;
  }

}

bool ViseServer::ReplaceString(std::string &s, std::string old_str, std::string new_str) {
  std::string::size_type pos = s.find(old_str);
  if ( pos == std::string::npos ) {
    return false;
  } else {
    s.replace(pos, old_str.length(), new_str);
    return true;
  }

}
