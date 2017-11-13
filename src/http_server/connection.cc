#include "connection.h"

const std::string Connection::crlf = "\r\n";
const std::string Connection::crlf2 = "\r\n\r\n";

const std::string Connection::http_100 = "HTTP/1.1 100 Continue\r\n";
const std::string Connection::http_200 = "HTTP/1.1 200 OK\r\n";
const std::string Connection::http_301 = "HTTP/1.1 301 Moved Permanently\r\n";
const std::string Connection::http_400 = "HTTP/1.1 400 Bad Request\r\n";
const std::string Connection::app_namespace = "vise";

Connection::Connection(boost::asio::io_service& io_service,
                       boost::filesystem::path upload_dir,
                       boost::filesystem::path result_dir): strand_( io_service ),
                                                            socket_( io_service ),
                                                            upload_dir_( upload_dir ),
                                                            result_dir_( result_dir )
{
  connection_id_ = boost::filesystem::unique_path("%%%%%%%%").string();
}

boost::asio::ip::tcp::socket& connection::socket() {
  return socket_;
}

void Connection::process_connection() {
  socket_.async_read_some(boost::asio::buffer( buffer_ ),
                          strand_.wrap( boost::bind(&connection::on_request_data, shared_from_this(),
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred
                                                    )
                                        )
                          );
}

void Connection::on_request_data(const boost::system::error_code& e, std::size_t bytes_read) {
    request_ << std::string( buffer_.data(), buffer_.data() + bytes_read );
    std::cout << "http request = {\n" << request_.str() << "}" << std::flush;
}

void Connection::on_response_write(const boost::system::error_code& e) {
  if ( !e ) {
    std::cerr << "Failed to send http response" << std::endl;
  }
  boost::system::error_code ignored_err;
  socket_.shutdown( boost::asio::ip::tcp::socket::shutdown_both, ignored_err );
}
