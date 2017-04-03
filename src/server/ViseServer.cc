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
    std::vector< std::string > request_key;
    std::vector< std::string > request_value;
    int line = 0;
    while ( ! httpstream.error() ) {
      //httpstream >> request;
      std::getline( httpstream, request );
      std::cout << "\n[ httprequest " << line << " ] : " << request << std::flush;

      if ( request.substr(0,3) == "GET" ) {
        unsigned int second_spc_idx = request.find( ' ', 4 );
        if ( second_spc_idx > 4 ) {
          std::string resource = request.substr(4, (second_spc_idx-4));

          std::vector< std::string > resource_tokens;
          SplitString(resource, '/', resource_tokens);

          std::string name = resource_tokens.at(1);
          resource_tokens.erase( resource_tokens.begin() );
          std::vector<std::string> param(resource_tokens);

          /*
          for (unsigned int j=0; j<resource_tokens.size(); ++j) {
            std::cout << "\nresource_tokens = " << resource_tokens.at(j);
          }
          */

          search_engine_.Init(name, vise_enginedir_);
          search_engine_.MoveToNextState();
        }
      }

      if ( request.substr(0,4) == "POST" ) {
        std::cout << "\nReceived POST request" << std::flush;
      }

      line = line + 1;
      error_ = httpstream.error();
      std::cout << "\nerror = " << error_.message() << std::flush;

    }

    //HandleConnection( socket );
  }
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
