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

  std::cout << "\nServer started on port " << port << " :-)";

  while ( 1 ) {
    boost::asio::ip::tcp::iostream httpstream;
    acceptor.accept( *httpstream.rdbuf() );

    boost::asio::ip::tcp::no_delay no_delay(true);
    httpstream.rdbuf()->set_option(no_delay);

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

      /*
        for (unsigned int j=0; j<tokens.size(); ++j) {
        std::cout << "\ntokens = " << tokens.at(j);
        }
      */

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

      /*
      SendHtml( "CONFIGURATION_OK", httpstream );
      httpstream.flush();
      */
      /*
      std::string redirect_uri = search_engine_.MoveToNextState();
      HttpRedirect(redirect_uri, httpstream);
      */

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
      std::cout << "\nsearch_engine_.GetResourceUri(\"settings\") = " << search_engine_.GetResourceUri("settings") << std::flush;
      if ( http_method_uri == search_engine_.GetResourceUri("settings") ) {
        // save vuse settings to a file
        while ( request != "\r" ) {
          std::getline( httpstream, request );
          std::cout << "\n\trequest data = " << request << std::flush;
        }

        //httpstream.expires_from_now(boost::posix_time::seconds(2));
        //httpstream.clear();
        //httpstream.rdbuf()->~basic_socket_streambuf();
        //httpstream.rdbuf()->non_blocking();
        //httpstream.rdbuf()->flush();
        httpstream << "HTTP/1.1 200 OK\r\n\r\n" << std::flush;

        // get the POST data
        std::string http_post_data = "";
        std::string line;
        std::cout << "\n\tGetting http put data : " << std::flush;
        while ( !httpstream.eof() ) {
          std::getline(httpstream, line);
          //httpstream >> line;
          http_post_data = http_post_data + line + "\n";
        }
        std::cout << "http_post_data = " << http_post_data << std::flush;

        std::string engine_config_path = search_engine_.GetEngineConfigPath().string();
        std::cout << "\nengine_config_path = " << engine_config_path << std::flush;
        WriteFile( engine_config_path, http_post_data );

        std::string redirect_uri = search_engine_.MoveToNextState();
        HttpRedirect(redirect_uri, httpstream);
      }
    }
  } // end of while (1) { ... }
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

void ViseServer::HttpRedirect( std::string redirect_uri,
                               boost::asio::ip::tcp::iostream &httpstream)
{
  httpstream << "HTTP/1.1 303 See Other\r\n";
  httpstream << "Location: " << (url_prefix_ + redirect_uri) << "\r\n";
  std::cout << "\nRedirecting to : " << (url_prefix_ + redirect_uri) << std::flush;
  httpstream.close();
}

void ViseServer::HandleRequest( std::string resource_name,
                                boost::asio::ip::tcp::iostream &httpstream)
{
  std::cout << "\nHandleRequest() : " << resource_name << std::flush;
  if ( resource_name == "settings" ) {
    // send the configure html
    if ( html_engine_settings_.length()  == 0 ) {
      LoadFile("/home/tlm/dev/vise/src/server/html_templates/settings.html", html_engine_settings_);
      std::string put_url = url_prefix_;
      put_url += search_engine_.GetResourceUri("settings");
      ReplaceString(html_engine_settings_,
                    "__VISE_SEARCH_ENGINE_SETTINGS_PUT_URL__",
                    put_url);
    }
    SendHtml( html_engine_settings_, httpstream );
    httpstream.close();
  } else if ( resource_name == "training" ) {
    // show the training status and progress page
    if ( html_engine_settings_.length()  == 0 ) {
      LoadFile("/home/tlm/dev/vise/src/server/html_templates/training.html", html_engine_training_);
    }
    SendHtml( html_engine_training_, httpstream );
    httpstream.close();
  } else {
    httpstream << "HTTP/1.1 200 OK\r\n";
    std::string result;
    result  = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">";
    result += "<title>VGG Image Search Engine</title></head>";
    result += "<body><h1>You seem to be lost.</h1>";
    result += "<p>It is good to be lost because it means that you were exploring.</p>";
    result += "</body></html>";

    std::time_t t = std::time(NULL);
    char date_str[100];
    std::strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %Z", std::gmtime(&t));
    httpstream << "Date: " << date_str << "\r\n";
    httpstream << "Content-type: text/html\r\n";
    httpstream << "Content-Length: " << result.length() << "\r\n";
    httpstream << "\r\n";
    httpstream << result;
    httpstream.close();
  }
}

void ViseServer::SendHtml(std::string html,
                          boost::asio::ip::tcp::iostream &httpstream )
{
    httpstream << "HTTP/1.1 200 OK\r\n";
    std::time_t t = std::time(NULL);
    char date_str[100];
    std::strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %Z", std::gmtime(&t));
    httpstream << "Date: " << date_str << "\r\n";
    httpstream << "Content-type: text/html\r\n";
    httpstream << "Content-Length: " << html.length() << "\r\n";
    httpstream << "\r\n";
    httpstream << html;
}

void ViseServer::HttpError(std::string error_code, boost::asio::ip::tcp::iostream &httpstream) {
  httpstream << "HTTP/1.1 404 Not Found\r\n";
  httpstream.close();
}

void ViseServer::LoadFile(std::string filename, std::string &file_contents) {
  std::ifstream f;
  f.open(filename.c_str());
  f.seekg(0, std::ios::end);
  file_contents.reserve( f.tellg() );
  f.seekg(0, std::ios::beg);

  file_contents.assign( std::istreambuf_iterator<char>(f),
                        std::istreambuf_iterator<char>() );
  f.close();
}

void ViseServer::WriteFile(std::string filename, std::string &file_contents) {
  std::ofstream f;
  f.open(filename.c_str());
  f << file_contents;
  f.close();
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
