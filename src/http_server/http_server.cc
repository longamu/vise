#include "http_server.h"

http_server::http_server(const std::string& address,
                         const std::string& port,
                         const std::size_t thread_pool_size,
                         const boost::filesystem::path upload_dir,
                         const boost::filesystem::path result_dir) : thread_pool_size_( thread_pool_size ),
                                                                     acceptor_( io_service_ ),
                                                                     signals_(io_service_),
                                                                     upload_dir_(upload_dir),
                                                                     result_dir_(result_dir)
{
  signals_.add(SIGINT);
  signals_.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
  signals_.async_wait(boost::bind(&http_server::stop, this));

  boost::asio::ip::tcp::resolver resolver( io_service_ );
  boost::asio::ip::tcp::resolver::query query( address, port );
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( query );

  acceptor_.open( endpoint.protocol() );
  acceptor_.set_option( boost::asio::ip::tcp::acceptor::reuse_address(true) );
  acceptor_.bind( endpoint );
  acceptor_.listen();

  accept_new_connection();
  std::clog << "\nhttp server waiting for connections at " << address
            << ":" << port << " (" << thread_pool_size << " threads)" << std::flush;
}

void http_server::start() {
  std::vector< boost::shared_ptr<boost::thread> > threads;
  for ( std::size_t i = 0; i < thread_pool_size_; ++i ) {
    // these threads will block until async operations are complete
    // the first async operation is created by a call to AcceptNewConnection() in http_server::http_server()
    boost::shared_ptr<boost::thread> connection_thread( new boost::thread( boost::bind( &boost::asio::io_service::run, &io_service_) ) );
    threads.push_back( connection_thread );
  }

  for ( std::size_t i = 0; i < threads.size(); ++i ) {
    threads[i]->join();
  }
}

void http_server::accept_new_connection() {
  new_connection_.reset( new connection(io_service_, upload_dir_, result_dir_) );
  acceptor_.async_accept( new_connection_->Socket(),
                          boost::bind(&http_server::handle_connection, this, boost::asio::placeholders::error) );
}

void http_server::handle_connection(const boost::system::error_code& e) {
  if ( !e ) {
    new_connection_->process_connection();
  }
  accept_new_connection();
}

void http_server::stop() {
  io_service_.stop();
}