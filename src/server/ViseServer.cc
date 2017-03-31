#include "ViseServer.h"

ViseServer::ViseServer() {
}

void ViseServer::Start(unsigned int port) {
  port_ = port;
  boost::asio::io_service io_service;
  tcp::acceptor acceptor ( io_service , tcp::endpoint( tcp::v4(), port_ ) );
  std::cout << "\nServer started on port " << port << " :-)";

  while ( 1 ) {
    tcp::socket socket( io_service );
    acceptor.accept( socket );

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
}

bool ViseServer::Stop() {
  std::cout << "\nServer stopped!";
  return true;
}
