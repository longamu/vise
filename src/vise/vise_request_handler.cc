#include "vise/vise_request_handler.h"

vise_request_handler *vise_request_handler::vise_request_handler_ = NULL;

vise_request_handler* vise_request_handler::instance() {
  if ( !vise_request_handler_ ) {
    vise_request_handler_ = new vise_request_handler;
  }
  return vise_request_handler_;
}

void vise_request_handler::init(const boost::filesystem::path data_dir,
                                const boost::filesystem::path asset_dir) {
  data_dir_ = data_dir;
  asset_dir_ = asset_dir;
  BOOST_LOG_TRIVIAL(debug) << "\nImageMagick Magick++ quantum depth = " << MAGICKCORE_QUANTUM_DEPTH << flush;
  BOOST_LOG_TRIVIAL(debug) << "\nvise_request_handler::init() : initializing http request handler" << flush;
  BOOST_LOG_TRIVIAL(debug) << "\nvise_request_handler::init() : data_dir=" << data_dir_.string() << flush;
  BOOST_LOG_TRIVIAL(debug) << "\nvise_request_handler::init() : asset_dir=" << asset_dir_.string() << flush;
}

void vise_request_handler::handle_http_request(const http_request& request, http_response& response) {
  BOOST_LOG_TRIVIAL(debug) << request.method_ << " [" << request.uri_ << "]";

  if ( request.method_ == "POST" ) {

  }

  // For the sake of completeness, vise includes a minimal (and possibly
  // inefficient) file content server.
  // In production environment, the GET requests should be handled by ngnix server.
  if ( request.method_ == "GET" ) {
    if ( vise::util::has_special_char(request.uri_) ) {
      response.set_status(400);
      BOOST_LOG_TRIVIAL(debug) << "http uri [" << request.uri_ << "] contains invalid character";
      return;
    }

    if ( vise::util::starts_with(request.uri_, "/vise/ui") ) {
      // serve files from vise/asset/vise/ui
      boost::filesystem::path fn = asset_dir_ / request.uri_;
      respond_with_static_file( response, fn );
      return;
    }

    if ( vise::util::starts_with(request.uri_, "/vise/search_engine_repo/") ) {
      if ( vise::util::contains(request.uri_, "/images/") ) {
        // serve files from vise/asset/vise/ui
        boost::filesystem::path fn = data_dir_ / request.uri_;
        respond_with_static_file( response, fn );
        return;
      }
    }
  }

  // all unhandled cases result in HTTP response 400 (Bad Request)
  response.set_status(400);
  return;
}

void vise_request_handler::respond_with_static_file(http_response& response, boost::filesystem::path fn) {
  std::string file_content;
  bool ok = vise::util::load_file_content(fn, file_content);
  if ( ok ) {
    response.set_payload( file_content );
    response.set_content_type_from_filename( fn.string() );
    BOOST_LOG_TRIVIAL(debug) << "http response contains file [" << fn.string() << "]";
  } else {
    response.set_status(400);
  }
}
