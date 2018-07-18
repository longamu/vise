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
  BOOST_LOG_TRIVIAL(debug) << "ImageMagick Magick++ quantum depth = " << MAGICKCORE_QUANTUM_DEPTH << flush;
  BOOST_LOG_TRIVIAL(debug) << "vise_request_handler::init() : initializing http request handler" << flush;
  BOOST_LOG_TRIVIAL(debug) << "vise_request_handler::init() : data_dir=" << data_dir_.string() << flush;
  BOOST_LOG_TRIVIAL(debug) << "vise_request_handler::init() : asset_dir=" << asset_dir_.string() << flush;
}

void vise_request_handler::handle_http_request(const http_request& request, http_response& response) {
  //BOOST_LOG_TRIVIAL(debug) << request.method_ << " [" << request.uri_ << "]";
  std::vector<std::string> uri_components;
  std::map<std::string, std::string> uri_param;
  vise::util::decompose_uri(request.uri_, uri_components, uri_param);
  //vise::util::print_vector("uri_components", uri_components);
  //vise::util::print_map("uri_param", uri_param);

  if ( request.method_ == "POST" ) {
    if ( vise::util::starts_with(request.uri_, "/vise/admin/") ) {
      if ( uri_components.size() < 3 ) {
        response.set_status(400);
        return;
      }
      if ( uri_components.size() != 6 || uri_components.size() != 4) {
        response.set_status(400);
        return;
      }
      if ( uri_components[1] != "vise" || uri_components[2] != "admin" ) {
        response.set_status(400);
        return;
      }

      // POST /vise/admin/_NAME_/_VERSION_/_COMMAND_
      if ( uri_components.size() == 6 ) {
        std::string search_engine_name    = uri_components[3];
        std::string search_engine_version = uri_components[4];
        std::string search_engine_command = uri_components[5];

        if ( vise::util::has_special_char(search_engine_name) ||
             vise::util::has_special_char(search_engine_version) ||
             vise::util::has_special_char(search_engine_command) ) {
          response.set_status(400);
          BOOST_LOG_TRIVIAL(debug) << "search engine uri [" << request.uri_ << "] contains invalid character";
          return;
        }

        search_engine_manager::instance()->process_cmd(search_engine_name,
                                                       search_engine_version,
                                                       search_engine_command,
                                                       uri_param,
                                                       request.payload_.str(),
                                                       response);
        return;
      }

      // POST /vise/admin/_COMMAND_
      if ( uri_components.size() == 4 ) {
        std::string admin_command = uri_components[3];

        if ( vise::util::has_special_char(admin_command) ) {
          response.set_status(400);
          BOOST_LOG_TRIVIAL(debug) << "admin command [" << request.uri_ << "] contains invalid character";
          return;
        }

        search_engine_manager::instance()->admin(admin_command,
                                                 uri_param,
                                                 request.payload_.str(),
                                                 response);
        return;
      }

      response.set_status(400);
      return;
    }
  }

  // For the sake of completeness, vise includes a minimal (and possibly inefficient) file content server.
  // In production environments, the GET requests should be handled by ngnix server.
  if ( request.method_ == "GET" ) {
    if ( vise::util::has_special_char(request.uri_) ) {
      response.set_status(400);
      BOOST_LOG_TRIVIAL(debug) << "http uri [" << request.uri_ << "] contains invalid character";
      return;
    }

    if ( vise::util::starts_with(request.uri_, "/vise/ui") ) {
      boost::filesystem::path fn = asset_dir_ / request.uri_;
      respond_with_static_file( response, fn );
      return;
    }

    if ( vise::util::starts_with(request.uri_, "/vise/file/") ) {
      // POST /vise/admin/_NAME_/_VERSION_/_COMMAND_
      if ( uri_components.size() > 5 ) {
        std::string search_engine_name    = uri_components[3];
        std::string search_engine_version = uri_components[4];
        std::string search_engine_file    = uri_components[5];

        if ( vise::util::has_special_char(search_engine_name) ||
             vise::util::has_special_char(search_engine_version) ||
             vise::util::has_special_char(search_engine_file) ) {
          response.set_status(400);
          BOOST_LOG_TRIVIAL(debug) << "search engine uri [" << request.uri_ << "] contains invalid character";
          return;
        }

        // @todo : respond with file contents
        boost::filesystem::path fn = data_dir_ / request.uri_;
        respond_with_static_file( response, fn );
        return;
      }
    }

    if ( vise::util::starts_with(request.uri_, "/vise/query/") ) {
      std::string search_engine_name    = uri_components[3];
      std::string search_engine_version = uri_components[4];
      std::string search_engine_query   = uri_components[5];

      if ( vise::util::has_special_char(search_engine_name) ||
           vise::util::has_special_char(search_engine_version) ||
           vise::util::has_special_char(search_engine_query) ) {
        response.set_status(400);
        BOOST_LOG_TRIVIAL(debug) << "search engine uri [" << request.uri_ << "] contains invalid character";
        return;
      }

      search_engine_manager::instance()->query(search_engine_name,
                                               search_engine_version,
                                               search_engine_query,
                                               uri_param,
                                               request.payload_.str(),
                                               response);
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
    BOOST_LOG_TRIVIAL(debug) << "failed to send file in http response [" << fn.string() << "]";
  }
}
