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

  /*
  while ( 1 ) {
    boost::asio::ip::tcp::iostream httpstream;

    acceptor.accept( *httpstream.rdbuf() );

    boost::asio::ip::tcp::no_delay no_delay(true);
    httpstream.rdbuf()->set_option(no_delay);

    boost::system::error_code ec = httpstream.rdbuf()->non_blocking(true, error_);
    std::cout << "\nerror_code = " << ec.message() << std::flush;
    if (ec) {
      std::cout << "\nNon-blocking mode" << std::flush;
    } else {
      std::cout << "\nBlocking-mode" << std::flush;
    }
    std::cout << "\nerror_ = " << error_.message() << std::flush;

    if (!httpstream) {
      std::cerr << "httpstream error: " << httpstream.error().message() << std::endl;
    }

    std::string request;
    std::vector<std::string> http_content;

    // extract the httpd method and resource name
    std::getline( httpstream, request );
    std::cout << "\nHTTP Request : " << request << std::flush;
    std::string http_method = request.substr(0, 4);
    std::vector<std::string> tokens;
    SplitString(request, ' ', tokens);
    if ( tokens.size() != 3 ) {
      std::cerr << "\nHTTP request contains unknown resource: " << request << std::flush;
      continue;
    }
    std::string http_method_uri = tokens.at(1);

    if ( http_method == "GET " ) {
      std::vector<std::string> tokens;
      SplitString( http_method_uri, '/', tokens );

      // (1) http://localhost:8080/some_name
      // (2) http://localhost:8080/some_name/
      bool init_search_engine = false;
      if ( tokens.at(0) == "" ) {
        if ( tokens.size() == 2 ) {
          init_search_engine = true;
        } else if ( tokens.size() == 3 && tokens.at(2) == "" ) {
          init_search_engine = true;
        }
      }

      if ( init_search_engine ) {

        std::string search_engine_name = tokens.at(1);
        search_engine_.Init(search_engine_name, vise_enginedir_);

        std::string redirect_uri = search_engine_.MoveToNextState();
        std::cout << "\nRedirecting to " << redirect_uri << std::flush;
        HttpRedirect(redirect_uri, httpstream);
      } else {
        HandleRequest(tokens.at(2), httpstream);
      }
    } else if ( http_method == "POST" ) {
      std::cout << "\nPOST : Resource = " << http_method_uri << std::flush;

      std::vector<std::string> tokens;
      SplitString( http_method_uri, '/', tokens );
      std::string post_resource = tokens.at(1);

      // skip past all the other headers
      unsigned int post_content_length = 0;
      while ( request != "\r" ) {
        std::getline( httpstream, request );
        //std::cout << "\nPost header = " << request << std::flush;
        if (request.substr(0, 14) == "Content-Length") {
          std::istringstream ss( request.substr(15) );
          ss >> post_content_length;
        }
      }


      httpstream.flush();
      // get the POST data
      std::string http_post_data = "";
      std::string line;
      while ( !httpstream.eof() ) {
        std::getline(httpstream, line);
        http_post_data = http_post_data + line + "\n";
      }
      std::cout << "http_post_data = " << http_post_data << std::flush;

      if ( post_resource == "set_search_engine_config" ) {
        std::string engine_config_path = search_engine_.GetEngineConfigPath().string();
        std::cout << "\nengine_config_path = " << engine_config_path << std::flush;
        WriteFile( engine_config_path, http_post_data );

        std::string redirect_uri = search_engine_.MoveToNextState();
        HttpRedirect(redirect_uri, httpstream);
        httpstream.close();
      }
    }  else if ( http_method == "PUT " ) {
      std::cout << "\nPUT : Resource = " << http_method_uri << std::flush;

      if ( http_method_uri == search_engine_.GetResourceUri("settings") ) {
        // save settings to a file
        while ( request != "\r" ) {
          std::getline( httpstream, request );
          std::cout << "\n  request data = " << request << std::flush;
        }

        //httpstream.expires_from_now(boost::posix_time::seconds(2));
        //httpstream.clear();
        //httpstream.rdbuf()->~basic_socket_streambuf();
        //httpstream.rdbuf()->non_blocking();
        //httpstream.rdbuf()->flush();
        //httpstream << "HTTP/1.1 200 OK\r\n\r\n" << std::flush;
        // SendHtml("OK", httpstream);

        // get the POST data
        std::string http_post_data = "";
        std::string line;
        std::cout << "\n\tGetting http put data : " << std::flush;

        std::cout << "\nhttpstream.rdbuf()->data()=" << httpstream.rdbuf() << std::flush;
        std::cout << "\nTest" << std::flush;
        //std::cout << "\nhttp_post_data=" << http_post_data << std::flush;

        std::string engine_config_path = search_engine_.GetEngineConfigPath().string();
        std::cout << "\nengine_config_path = " << engine_config_path << std::flush;
        WriteFile( engine_config_path, http_post_data );

        std::string redirect_uri = search_engine_.MoveToNextState();
        HttpRedirect(redirect_uri, httpstream);
      }
    }
  } // end of while (1) { ... }
  */
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

  if ( http_method == "GET " ) {
    // (1) http://localhost:8080/some_name
    // (2) http://localhost:8080/some_name/
    bool init_search_engine = false;
    if ( tokens.at(0) == "" ) {
      if ( tokens.size() == 2 ) {
        init_search_engine = true;
      } else if ( tokens.size() == 3 && tokens.at(2) == "" ) {
        init_search_engine = true;
      }
    }

    if ( init_search_engine ) {
      std::string search_engine_name = tokens.at(1);
      search_engine_.Init(search_engine_name, vise_enginedir_);

      std::string redirect_uri = search_engine_.MoveToNextState();
      SendHttpRedirect(redirect_uri, p_socket);
    } else {
      HandleRequest(tokens.at(2), p_socket);
    }
  } else if ( http_method == "POST" ) {
    // read
    std::string http_post_data;
    ExtractHttpContent(http_request, http_post_data);

    // evaluate
    std::string engine_config_path = search_engine_.GetEngineConfigPath().string();
    WriteFile( engine_config_path, http_post_data );

    // reply (or redirect)
    std::string redirect_uri = search_engine_.MoveToNextState();
    SendHttpRedirect(redirect_uri, p_socket);
  } else {
    SendHttpNotFound( p_socket );
  }
  p_socket->close();
}


void ViseServer::HandleRequest( std::string resource_name,
                                boost::shared_ptr<tcp::socket> p_socket)
{
  std::cout << "\nHandleRequest() : " << resource_name << std::flush;
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
    if ( html_engine_training_.length()  == 0 ) {
      LoadFile("/home/tlm/dev/vise/src/server/html_templates/training.html", html_engine_training_);
    }
    SendHttpResponse( html_engine_training_, p_socket );
  } else {
    SendHttpNotFound(p_socket);
  }
}

void ViseServer::SendHttpResponse(std::string html, boost::shared_ptr<tcp::socket> p_socket) {
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
  http_response << "Content-Length: " << html.length() << "\r\n";
  http_response << "\r\n";
  http_response << html;
  boost::asio::write( *p_socket, boost::asio::buffer(http_response.str()) );

  std::cout << "\nSent http response of length : " << html.length() << std::flush;
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

void ViseServer::LoadFile(std::string filename, std::string &file_contents) {
  try {
    std::ifstream f;
    f.open(filename.c_str());
    f.seekg(0, std::ios::end);
    file_contents.reserve( f.tellg() );
    f.seekg(0, std::ios::beg);

    file_contents.assign( std::istreambuf_iterator<char>(f),
                          std::istreambuf_iterator<char>() );
    f.close();
  } catch (std::exception &e) {
    std::cerr << "\nViseServer::LoadFile() : failed to load file : " << filename << std::flush;
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
