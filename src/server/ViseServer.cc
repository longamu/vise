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
  port_ = port;
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::endpoint endpoint( tcp::v4(), port_ );
  tcp::acceptor acceptor ( io_service , endpoint );

  std::cout << "\nServer started on port " << port << " :-)";

  while ( 1 ) {
    boost::asio::ip::tcp::iostream httpstream;
    acceptor.accept( *httpstream.rdbuf() );

    if (!httpstream) {
      std::cerr << "httpstream error: " << httpstream.error().message() << std::endl;
    }

    std::string request;
    std::vector<std::string> http_content;

    // extract the httpd method and resource name
    std::getline( httpstream, request );
    std::string http_method = request.substr(0, 4);
    std::vector<std::string> tokens;
    SplitString(request, ' ', tokens);
    if ( tokens.size() != 3 ) {
      std::cerr << "\nHTTP request contains unknown resource: " << request << std::flush;
      continue;
    }
    std::string http_method_uri = tokens.at(1);

    if ( http_method == "GET " ) {
      std::cout << "\nGET : Resource = " << http_method_uri << std::flush;

      std::vector<std::string> tokens;
      SplitString( http_method_uri, '/', tokens );

      /*
      for (unsigned int j=0; j<tokens.size(); ++j) {
        std::cout << "\ntokens = " << tokens.at(j);
      }
      */

      std::string search_engine_name = tokens.at(1);
      search_engine_.Init(search_engine_name, vise_enginedir_);
      std::cout << "\nsearch_engine_name = " << search_engine_name;
      std::string result;
      search_engine_.MoveToNextState( result );
      httpstream << "HTTP/1.0 200 OK\r\n";
      std::time_t t = std::time(NULL);
      char date_str[100];
      std::strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %Z", std::gmtime(&t));
      httpstream << "Date: " << date_str << "\r\n";
      httpstream << "Content-type: text/html\r\n";
      httpstream << "Content-Length: " << result.length() << "\r\n";
      httpstream << "\r\n";
      httpstream << result;
      httpstream.close();
    } else if ( http_method == "POST" ) {
      std::cout << "\nPOST : Resource = " << http_method_uri << std::flush;

      // skip past all the other headers
      unsigned int post_content_length = 0;
      while ( request != "\r" ) {
        std::getline( httpstream, request );
        std::cout << "\nPost header = " << request << std::flush;
        if (request.substr(0, 14) == "Content-Length") {
          std::istringstream ss( request.substr(15) );
          ss >> post_content_length;
        }
      }

      std::string result = "CONFIGURATION_OK";
      httpstream << "HTTP/1.0 200 OK\r\n";
      std::time_t t = std::time(NULL);
      char date_str[100];
      std::strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %Z", std::gmtime(&t));
      httpstream << "Date: " << date_str << "\r\n";
      httpstream << "Content-type: text/html\r\n";
      httpstream << "Content-Length: " << result.length() << "\r\n";
      httpstream << "\r\n";
      httpstream << result;
      httpstream.flush();

      // get the POST data
      std::string http_post_data;
      std::cout << "http_post_data = \"" << std::flush;
      while ( !httpstream.eof() ) {
        httpstream >> http_post_data;
        std::cout << http_post_data << " " << std::flush;
      }
      std::cout << "\"" << std::flush;
    }
  } // end of while (1) { ... }
}

void ViseServer::HandleConnection(boost::asio::ip::tcp::socket socket) {
  boost::array<char, 1024> buf;
  size_t request_len = socket.read_some( boost::asio::buffer(buf), error_ );
  std::cout << "\nRequest = \n";
  std::cout.write(buf.data(), request_len);

  std::string message = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"></head><body><h1>Hello World!</h1></body></html>";

  std::ostringstream message_len;
  message_len << message.length();

  std::string header = "HTTP/1.0 200 OK\r\nDate: Fri, 31 Dec 1999 23:59:59 GMT\r\nContent-Type: text/html\r\nContent-Length: " + message_len.str() + "\r\n\r\n";

  std::string reply = header + message;
  boost::asio::write( socket, boost::asio::buffer(reply), error_ );

  if ( error_ ) {
    std::cout << "\n  - Error sending reply : " << error_.message() << std::flush;
  } else {
    std::cout << "\nResponse = \n" << reply << std::flush;
  }
  socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
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
