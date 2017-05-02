#include "ViseServer.h"

const int ViseServer::STATE_NOT_LOADED;
const int ViseServer::STATE_SETTING;
const int ViseServer::STATE_INFO;
const int ViseServer::STATE_PREPROCESS;
const int ViseServer::STATE_DESCRIPTOR;
const int ViseServer::STATE_CLUSTER;
const int ViseServer::STATE_ASSIGN;
const int ViseServer::STATE_HAMM;
const int ViseServer::STATE_INDEX;
const int ViseServer::STATE_QUERY;

ViseServer::ViseServer( std::string vise_datadir, std::string vise_templatedir ) {
  // set resource names
  vise_datadir_      = boost::filesystem::path(vise_datadir);
  vise_templatedir_  = boost::filesystem::path(vise_templatedir);

  if ( ! boost::filesystem::exists( vise_datadir_ ) ) {
    std::cerr << "\nViseServer::ViseServer() : vise_datadir_ does not exist! : "
              << vise_datadir_.string() << std::flush;
    return;
  }
  if ( ! boost::filesystem::exists( vise_templatedir_ ) ) {
    std::cerr << "\nViseServer::ViseServer() : vise_templatedir_ does not exist! : "
              << vise_templatedir_.string() << std::flush;
    return;
  }

  vise_enginedir_    = vise_datadir_ / "search_engines";
  if ( ! boost::filesystem::exists( vise_enginedir_ ) ) {
    boost::filesystem::create_directory( vise_enginedir_ );
  }

  vise_logdir_       = vise_datadir_ / "log";
  if ( ! boost::filesystem::exists( vise_logdir_ ) ) {
    boost::filesystem::create_directory( vise_logdir_ );
  }

  vise_main_html_fn_ = vise_templatedir_ / "vise_main.html";
  vise_help_html_fn_ = vise_templatedir_ / "vise_help.html";
  vise_css_fn_       = vise_templatedir_ / "vise.css";
  vise_js_fn_        = vise_templatedir_ / "vise.js";
  vise_404_html_fn_  = vise_templatedir_ / "404.html";

  // load html resources
  LoadFile(vise_main_html_fn_.string(), vise_main_html_);
  LoadFile(vise_help_html_fn_.string(), vise_help_html_);
  LoadFile(vise_404_html_fn_.string() , vise_404_html_);
  LoadFile(vise_css_fn_.string(), vise_css_);
  LoadFile(vise_js_fn_.string() , vise_js_);

  state_id_list_.push_back( ViseServer::STATE_NOT_LOADED );
  state_name_list_.push_back( "NotLoaded" );
  state_html_fn_list_.push_back( "404.html" );
  state_info_list_.push_back( "" );

  state_id_list_.push_back( ViseServer::STATE_SETTING );
  state_name_list_.push_back( "Setting" );
  state_html_fn_list_.push_back( "Setting.html" );
  state_info_list_.push_back( "" );

  state_id_list_.push_back( ViseServer::STATE_INFO );
  state_name_list_.push_back( "Info" );
  state_html_fn_list_.push_back( "Info.html" );
  state_info_list_.push_back( "(2 min.)" );

  state_id_list_.push_back( ViseServer::STATE_PREPROCESS );
  state_name_list_.push_back( "Preprocess" );
  state_html_fn_list_.push_back( "Preprocess.html" );
  state_info_list_.push_back( "(6 min.)" );

  state_id_list_.push_back( ViseServer::STATE_DESCRIPTOR );
  state_name_list_.push_back( "Descriptor" );
  state_html_fn_list_.push_back( "Descriptor.html" );
  state_info_list_.push_back( "(49 min.)" );

  state_id_list_.push_back( ViseServer::STATE_CLUSTER );
  state_name_list_.push_back( "Cluster" );
  state_html_fn_list_.push_back( "Cluster.html" );
  state_info_list_.push_back( "(12 min.)" );

  state_id_list_.push_back( ViseServer::STATE_ASSIGN );
  state_name_list_.push_back( "Assign" );
  state_html_fn_list_.push_back( "Assign.html" );
  state_info_list_.push_back( "(1 min.)" );

  state_id_list_.push_back( ViseServer::STATE_HAMM );
  state_name_list_.push_back( "Hamm" );
  state_html_fn_list_.push_back( "Hamm.html" );
  state_info_list_.push_back( "(1 min.)" );

  state_id_list_.push_back( ViseServer::STATE_INDEX );
  state_name_list_.push_back( "Index" );
  state_html_fn_list_.push_back( "Index.html" );
  state_info_list_.push_back( "(3 hours)" );

  state_id_list_.push_back( ViseServer::STATE_QUERY );
  state_name_list_.push_back( "Query" );
  state_html_fn_list_.push_back( "Query.html" );
  state_info_list_.push_back( "" );

  state_id_ = ViseServer::STATE_NOT_LOADED;

  // load all state html
  state_html_list_.resize( state_id_list_.size() );
  for (unsigned int i=0; i < state_id_list_.size(); i++) {
    state_html_list_.at(i) = "";
    boost::filesystem::path fn = vise_templatedir_ / state_html_fn_list_.at(i);
    LoadFile( fn.string(), state_html_list_.at(i) );
  }

  vise_index_html_reload_ = true;

  // open a log file to save training statistics
  // for logging statistics
  vise_training_stat_fn_ = vise_logdir_ / "training_stat.csv";

  bool append_csv_header = false;
  if ( ! boost::filesystem::exists( vise_training_stat_fn_ ) ) {
    append_csv_header = true;
  }

  training_stat_f.open( vise_training_stat_fn_.string().c_str(), std::ofstream::app );
  if ( append_csv_header ) {
    training_stat_f << "date,time,dataset_name,state_name,time_sec,space_bytes";
  }
  std::cout << "\nvise_training_stat_fn_ = " << vise_training_stat_fn_ << std::flush;
}

ViseServer::~ViseServer() {
  // for logging statistics
  training_stat_f.close();
}


void ViseServer::Start(unsigned int port) {
  hostname_ = "localhost";
  port_ = port;

  std::ostringstream url_builder;
  url_builder << "http://" << hostname_ << ":" << port_;
  url_prefix_ = url_builder.str();

  boost::asio::io_service io_service;
  boost::asio::ip::tcp::endpoint endpoint( tcp::v4(), port_ );
  tcp::acceptor acceptor ( io_service , endpoint );
  acceptor.set_option( tcp::acceptor::reuse_address(true) );

  std::cout << "\nServer started on port " << port << " :-)" << std::flush;

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
  std::cout << "\n" << http_method << " " << http_method_uri << std::flush;

  if ( http_method == "GET " ) {
    if ( http_method_uri == "/" ) {
      // show help page when user enteres http://localhost:8080
      SendHttpResponse( vise_main_html_, p_socket);
      p_socket->close();
      return;
    }

    if ( http_method_uri == "/_vise_index.html" ) {
      if ( vise_index_html_reload_ ) {
        GenerateViseIndexHtml();
      }
      SendHttpResponse(vise_index_html_, p_socket);
      p_socket->close();
      return;
    }

    if ( http_method_uri == "/_state" ) {
      // @todo
      // json reply containing current state info.
      std::string state_json = GetStateJsonData();
      SendJsonResponse( state_json, p_socket );
      p_socket->close();
      return;
    }

    if ( http_method_uri == "/vise.css" ) {
      SendRawResponse( "text/css", vise_css_, p_socket );
      p_socket->close();
      return;
    }

    if ( http_method_uri == "/vise.js" ) {
      SendRawResponse( "application/javascript", vise_js_, p_socket );
      p_socket->close();
      return;
    }

    if ( http_method_uri.length() > 8 &&
         http_method_uri.substr( http_method_uri.length() - 8 ) == "_message" ) {
      // since HTTP server always requires a request in order to send responses,
      // we always create this _message channel which keeps waiting for messages
      // to be pushed to vise_message_queue_, sends this message to the client
      // which in turn again creates another request for any future messages
      std::string msg;
      vise_message_queue_.BlockingPop( msg );
      SendRawResponse( "text/plain", msg, p_socket );
      p_socket->close();
      return;
    }

    // if http_method_uri contains request for static resource in the form:
    //   GET /_static/ox5k/img/dir1/dir2/all_souls_000022.jpg?original
    // return the static resource as binary data in HTTP response
    const std::string static_resource_prefix = "/_static";
    if ( StringStartsWith(http_method_uri, static_resource_prefix) ) {
      std::string resource_uri = http_method_uri.substr( static_resource_prefix.size(), std::string::npos);
      std::string resource_arg = "";

      std::size_t qmark_pos = http_method_uri.find('?');
      if ( qmark_pos != std::string::npos ) {
        std::string temp = resource_uri;
        resource_uri = resource_uri.substr(0, qmark_pos);
        resource_arg =resource_uri.substr(qmark_pos+1, std::string::npos);
      }
      ServeStaticResource( resource_uri, resource_arg, p_socket );
      p_socket->close();
      return;
    }

    // request for state html
    // Get /Cluster
    std::vector< std::string > tokens;
    SplitString( http_method_uri, '/', tokens);
    if ( tokens.at(0) == "" && tokens.size() == 2 ) {
      const std::string resource_name = tokens.at(1);
      int state_id = GetStateId( resource_name );
      if ( state_id != -1 ) {
        HandleStateGetRequest( state_id, p_socket );
        p_socket->close();
        return;
      }
    }

    // otherwise, say not found
    SendHttp404NotFound( p_socket );
    p_socket->close();
    return;
  }
  if ( http_method == "POST" ) {
    std::string http_post_data;
    ExtractHttpContent(http_request, http_post_data);

    if ( http_method_uri == "/" ) {
      // the http_post_data can be one of these:
      //  * create_search_engine _NAME_OF_SEARCH_ENGINE_
      //  * load_search_engine   _NAME_OF_SEARCH_ENGINE_
      std::vector< std::string > tokens;
      SplitString( http_post_data, ' ', tokens);
      if ( tokens.size() == 2 ) {
        std::string search_engine_name = tokens.at(1);
        if ( tokens.at(0) == "create_search_engine" ) {
          search_engine_.Init( search_engine_name, vise_enginedir_ );
          if ( UpdateState() ) {
            // send control message : state updated
            SendCommand("_state update_now");
            SendHttpPostResponse( http_post_data, "OK", p_socket );
          } else {
            SendHttpPostResponse( http_post_data, "ERR", p_socket );
          }
          p_socket->close();
          return;
          // send control message to set loaded engine name
        } else if ( tokens.at(0) == "load_search_engine" ) {
          std::cout << "\nLoading engine: " << search_engine_name << std::flush;
          LoadSearchEngine( search_engine_name );
          SendHttpPostResponse( http_post_data, "OK", p_socket );
          p_socket->close();
          return;
        } else {
          // unknown command
          SendHttp404NotFound( p_socket );
          p_socket->close();
          return;
        }
      } else {
        // unexpected POST data
        SendHttp404NotFound( p_socket );
        p_socket->close();
        return;
      }
    } else {
      std::string resource_name = http_method_uri.substr(1, std::string::npos); // remove prefix "/"
      int state_id = GetStateId( resource_name );
      if ( state_id != -1 ) {
        HandleStatePostData( state_id, http_post_data, p_socket );
        p_socket->close();
        return;
      }
      // otherwise, say not found
      SendHttp404NotFound( p_socket );
      p_socket->close();
      return;
    }
  }
}


//
// Communications with HTTP client
//
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

void ViseServer::SendRawResponse(std::string content_type,
                                 std::string content,
                                 boost::shared_ptr<tcp::socket> p_socket) {
  std::stringstream http_response;
  http_response << "HTTP/1.1 200 OK\r\n";
  http_response << "Content-type: " << content_type << "; charset=utf-8\r\n";
  http_response << "Content-Length: " << content.length() << "\r\n";
  http_response << "\r\n";
  http_response << content;

  boost::asio::write( *p_socket, boost::asio::buffer(http_response.str()) );
  //std::cout << "\nSent response : [" << response << "]" << std::flush;
}

void ViseServer::SendJsonResponse(std::string json, boost::shared_ptr<tcp::socket> p_socket) {
  std::stringstream http_response;
  http_response << "HTTP/1.1 200 OK\r\n";
  http_response << "Content-type: application/json; charset=utf-8\r\n";
  http_response << "Content-Length: " << json.length() << "\r\n";
  http_response << "Connection: keep-alive\r\n";
  http_response << "\r\n";
  http_response << json;

  boost::asio::write( *p_socket, boost::asio::buffer(http_response.str()) );
  //std::cout << "\nSent json response : [" << json << "]" << std::flush;
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

  //std::cout << "\nSent http html response of length : " << response.length() << std::flush;
}

void ViseServer::SendImageResponse(boost::filesystem::path im_fn, boost::shared_ptr<tcp::socket> p_socket) {
  std::string content_type = GetHttpContentType(im_fn);
  std::stringstream http_response;
  http_response << "HTTP/1.1 200 OK\r\n";
  std::time_t t = std::time(NULL);
  char date_str[100];
  std::strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %Z", std::gmtime(&t));
  http_response << "Date: " << date_str << "\r\n";
  http_response << "Content-Language: en\r\n";
  http_response << "Connection: close\r\n";

  http_response << "Content-type: " << content_type << "\r\n";
  http_response << "Content-Encoding: utf-8\r\n";
  http_response << "Content-Length: " << boost::filesystem::file_size( im_fn ) << "\r\n";
  http_response << "\r\n";
  boost::asio::write( *p_socket, boost::asio::buffer(http_response.str()) );

  // write the image file contents to socket
  try {
    std::ifstream im( im_fn.string().c_str(), std::ios::binary );
    if ( im.is_open() ) {
      const unsigned int BUF_SIZE = 1024 * 1024; // 1024 KB
      char* im_buf = new char[BUF_SIZE];
      while ( !im.eof() ) {
        memset(im_buf, 0, BUF_SIZE);
        im.read(im_buf, BUF_SIZE);
        unsigned int bytes_read = im.gcount();
        boost::asio::write( *p_socket,
                            boost::asio::buffer(im_buf, bytes_read),
                            boost::asio::transfer_all());
      }
      im.close();
      delete( im_buf);
    }
  } catch( std::exception &e) {
    std::cerr << "\nViseServer::SendImageResponse() : exception " << e.what() << std::flush;
  }

}

void ViseServer::SendMessage(std::string message) {
  SendPacket( "message", message);
}

void ViseServer::SendLog(std::string log) {
  SendPacket( "log", log );
}

void ViseServer::SendCommand(std::string command) {
  SendPacket("command", command );
}

void ViseServer::SendPacket(std::string type, std::string message) {
  std::ostringstream s;
  s << GetCurrentStateName() << " " << type << " " << message;
  vise_message_queue_.Push( s.str() );
}

void ViseServer::SendHttpPostResponse(std::string http_post_data, std::string result, boost::shared_ptr<tcp::socket> p_socket) {
  std::string response = "{ \"id\": \"http_post_response\",";
  response += "\"http_post_data\": \"" + http_post_data + "\",";
  response += "\"result\": \"OK\" }";

  std::ostringstream http_response;
  http_response << "HTTP/1.1 200 OK\r\n";
  std::time_t t = std::time(NULL);
  char date_str[100];
  std::strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %Z", std::gmtime(&t));
  http_response << "Date: " << date_str << "\r\n";
  http_response << "Connection: Closed\r\n";
  http_response << "Content-type: application/json\r\n";
  http_response << "Content-Length: " << response.length() << "\r\n";
  http_response << "\r\n";
  http_response << response;

  boost::asio::write( *p_socket, boost::asio::buffer(http_response.str()) );
}

void ViseServer::SendHttp404NotFound(boost::shared_ptr<tcp::socket> p_socket) {
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

//
// Handle HTTP Get requests (used to query search engine)
//
void ViseServer::HandleStateGetRequest( int state_id,
                                        boost::shared_ptr<tcp::socket> p_socket ) {
  switch( state_id ) {
  case ViseServer::STATE_INFO:
    search_engine_.UpdateEngineOverview();
    state_html_list_.at( ViseServer::STATE_INFO ) = search_engine_.GetEngineOverview();
    break;
  case ViseServer::STATE_QUERY:
    // @todo-query
    // dynamically prepare a html page to show the results of query
    break;
  }

  SendHttpResponse( state_html_list_.at(state_id), p_socket );
}

void ViseServer::ServeStaticResource(const std::string resource_uri,
                                     const std::string resource_arg,
                                     boost::shared_ptr<tcp::socket> p_socket) {
  // resource uri format:
  // SEARCH_ENGINE_NAME/...path...
  std::cout << "\nViseServer::ServeStaticResource() : " << resource_uri << ", " << resource_arg << std::flush;
  std::size_t slash_pos = resource_uri.find('/');
  if ( slash_pos != std::string::npos ) {
    std::string search_engine_name = resource_uri.substr(0, slash_pos);
    if ( search_engine_.GetName() == search_engine_name ) {
      std::string res_rel_path = resource_uri.substr(slash_pos+1, std::string::npos);
      boost::filesystem::path res_fn = search_engine_.GetTransformedImageDir() / res_rel_path;
      if ( resource_arg == "original" ) {
        res_fn = search_engine_.GetOriginalImageDir() / res_rel_path;
      }
      std::cout << "\nViseServer::ServeStaticResource() : Serving static resource : " << res_fn.string() << std::flush;
      if ( boost::filesystem::exists(res_fn) ) {
        SendImageResponse( res_fn, p_socket );
      }
    }
  }

  // somethign was wrong
  std::ostringstream s;
  s << "\nViseServer::ServeStaticResource() : cannot serve static resource ";
  s << "[" << resource_uri << "," << resource_arg << "]";
  SendHttpResponse( s.str(), p_socket);
  return;
}

//
// Handler of HTTP POST data for States
//
void ViseServer::HandleStatePostData( int state_id, std::string http_post_data, boost::shared_ptr<tcp::socket> p_socket ) {
  if ( state_id == ViseServer::STATE_SETTING ) {
    search_engine_.SetEngineConfig(http_post_data);

    if ( UpdateState() ) {
      // send control message : state updated
      SendCommand("_state update_now");
    }
  } else if ( state_id == ViseServer::STATE_INFO ) {
    if ( http_post_data == "proceed" ) {
      if ( UpdateState() ) {
        // send control message : state updated
        SendCommand("_state update_now");
        SendHttpPostResponse( http_post_data, "OK", p_socket );

        // initiate the search engine training process
        boost::thread t( boost::bind( &ViseServer::InitiateSearchEngineTraining, this ) );
      } else {
        SendHttpPostResponse( http_post_data, "ERR", p_socket );
      }
    }
  } else {
    std::cerr << "\nViseServer::HandleStatePostData() : Do not know how to handle this HTTP POST data!" << std::flush;
  }
}

//
// Search engine training
//
void ViseServer::InitiateSearchEngineTraining() {
  SendCommand("_log clear show");

  // Pre-process
  if ( state_id_ == ViseServer::STATE_PREPROCESS ) {
    boost::timer::cpu_timer t_start;
    search_engine_.Preprocess();
    boost::timer::cpu_times elapsed = t_start.elapsed();

    AddTrainingStat(search_engine_.GetName(),
                    GetCurrentStateName(),
                    elapsed.wall / 1e9,
                    search_engine_.GetImglistTransformedSize());

    if ( UpdateState() ) {
      // send control message : state updated
      SendCommand("_state update_now");
    } else {
      SendMessage("\n" + GetCurrentStateName() + " : failed to change to next state");
      return;
    }
  }

  // Descriptor
  if ( state_id_ == ViseServer::STATE_DESCRIPTOR ) {
    boost::timer::cpu_timer t_start;
    search_engine_.Descriptor();
    boost::timer::cpu_times elapsed = t_start.elapsed();

    AddTrainingStat(search_engine_.GetName(),
                    GetCurrentStateName(),
                    elapsed.wall / 1e9,
                    search_engine_.DescFnSize());

    if ( UpdateState() ) {
      // send control message : state updated
      SendCommand("_state update_now");
    } else {
      SendMessage("\n" + GetCurrentStateName() + " : failed to change to next state");
      return;
    }
  }

  // Cluster
  if ( state_id_ == ViseServer::STATE_CLUSTER ) {
    boost::timer::cpu_timer t_start;
    search_engine_.Cluster();
    boost::timer::cpu_times elapsed = t_start.elapsed();

    AddTrainingStat(search_engine_.GetName(),
                    GetCurrentStateName(),
                    elapsed.wall / 1e9,
                    search_engine_.ClstFnSize());

    if ( UpdateState() ) {
      // send control message : state updated
      SendCommand("_state update_now");
    } else {
      SendMessage("\n" + GetCurrentStateName() + " : failed to change to next state");
      return;
    }
  }

  // Assign
  if ( state_id_ == ViseServer::STATE_ASSIGN ) {
    boost::timer::cpu_timer t_start;
    search_engine_.Assign();
    boost::timer::cpu_times elapsed = t_start.elapsed();

    AddTrainingStat(search_engine_.GetName(),
                    GetCurrentStateName(),
                    elapsed.wall / 1e9,
                    search_engine_.AssignFnSize());

    if ( UpdateState() ) {
      // send control message : state updated
      SendCommand("_state update_now");
    } else {
      SendMessage("\n" + GetCurrentStateName() + " : failed to change to next state");
      return;
    }
  }

  // Hamm
  if ( state_id_ == ViseServer::STATE_HAMM ) {
    boost::timer::cpu_timer t_start;
    search_engine_.Hamm();
    boost::timer::cpu_times elapsed = t_start.elapsed();

    AddTrainingStat(search_engine_.GetName(),
                    GetCurrentStateName(),
                    elapsed.wall / 1e9,
                    search_engine_.HammFnSize());

    if ( UpdateState() ) {
      // send control message : state updated
      SendCommand("_state update_now");
    } else {
      SendMessage("\n" + GetCurrentStateName() + " : failed to change to next state");
      return;
    }
  }

  // Index
  if ( state_id_ == ViseServer::STATE_INDEX ) {
    boost::timer::cpu_timer t_start;
    search_engine_.Index();
    boost::timer::cpu_times elapsed = t_start.elapsed();

    AddTrainingStat(search_engine_.GetName(),
                    GetCurrentStateName(),
                    elapsed.wall / 1e9,
                    search_engine_.IndexFnSize());

    if ( UpdateState() ) {
      // send control message : state updated
      SendCommand("_state update_now");
    } else {
      SendMessage("\n" + GetCurrentStateName() + " : failed to change to next state");
      return;
    }
  }
}

// State based model
//
std::string ViseServer::GetCurrentStateName() {
  return GetStateName( state_id_ );
}

std::string ViseServer::GetCurrentStateInfo() {
  return GetStateInfo( state_id_ );
}

int ViseServer::GetStateId( std::string state_name ) {
  for ( unsigned int i=0; i < state_id_list_.size(); i++) {
    if ( state_name_list_.at(i) == state_name ) {
      return state_id_list_.at(i);
    }
  }
  return -1; // not found
}

std::string ViseServer::GetStateName(int state_id) {
  return state_name_list_.at( state_id );
}

std::string ViseServer::GetStateInfo(int state_id) {
  return state_info_list_.at( state_id );
}

int ViseServer::GetCurrentStateId() {
  return state_id_;
}

bool ViseServer::UpdateState() {
  if ( state_id_ == ViseServer::STATE_NOT_LOADED ) {
    // check if search engine was initialized properly
    if ( search_engine_.GetName() == "" ) {
      return false;
    } else {
      state_id_ = ViseServer::STATE_SETTING;
      return true;
    }
  } else if ( state_id_ == ViseServer::STATE_SETTING ) {
    if ( search_engine_.IsEngineConfigEmpty() ) {
      return false;
    } else {
      state_id_ = ViseServer::STATE_INFO;
      return true;
    }
  } else if ( state_id_ == ViseServer::STATE_INFO ) {
    state_id_ = ViseServer::STATE_PREPROCESS;
    return true;
  } else if ( state_id_ == ViseServer::STATE_PREPROCESS ) {
    if ( search_engine_.EngineConfigFnExists() &&
         search_engine_.ImglistFnExists() ) {
      state_id_ = ViseServer::STATE_DESCRIPTOR;
      return true;
    } else {
      return false;
    }
  } else if ( state_id_ == ViseServer::STATE_DESCRIPTOR ) {
    if ( search_engine_.DescFnExists() ) {
      state_id_ = ViseServer::STATE_CLUSTER;
      return true;
    } else {
      return false;
    }
  } else if ( state_id_ == ViseServer::STATE_CLUSTER ) {
    if ( search_engine_.ClstFnExists() ) {
      state_id_ = ViseServer::STATE_ASSIGN;
      return true;
    } else {
      return false;
    }
  } else if ( state_id_ == ViseServer::STATE_ASSIGN ) {
    if ( search_engine_.AssignFnExists() ) {
      state_id_ = ViseServer::STATE_HAMM;
      return true;
    } else {
      return false;
    }
  } else if ( state_id_ == ViseServer::STATE_HAMM ) {
    if ( search_engine_.HammFnExists() ) {
      state_id_ = ViseServer::STATE_INDEX;
      return true;
    } else {
      return false;
    }
  } else if ( state_id_ == ViseServer::STATE_INDEX ) {
    if ( search_engine_.IndexFnExists() ) {
      state_id_ = ViseServer::STATE_QUERY;
      return true;
    } else {
      return false;
    }
  }
  return false;
}

void ViseServer::LoadSearchEngine( std::string search_engine_name ) {
  SendCommand("_log clear show");
  search_engine_.Init( search_engine_name, vise_enginedir_ );

  std::string engine_config;
  LoadFile( search_engine_.GetEngineConfigFn().string(), engine_config );
  search_engine_.SetEngineConfig( engine_config );
  if ( !UpdateState() ) {
    return;
  }
  assert( GetCurrentStateId() == ViseServer::STATE_SETTING );

  if ( !UpdateState() ) {
    return;
  }
  assert( GetCurrentStateId() == ViseServer::STATE_INFO );

  search_engine_.Preprocess();
  if ( !UpdateState() ) {
    return;
  }
  assert( GetCurrentStateId() == ViseServer::STATE_PREPROCESS );

  search_engine_.Descriptor();
  if ( !UpdateState() ) {
    return;
  }
  assert( GetCurrentStateId() == ViseServer::STATE_DESCRIPTOR );

  search_engine_.Cluster();
  if ( !UpdateState() ) {
    return;
  }
  assert( GetCurrentStateId() == ViseServer::STATE_CLUSTER );

  search_engine_.Assign();
  if ( !UpdateState() ) {
    return;
  }
  assert( GetCurrentStateId() == ViseServer::STATE_ASSIGN );

  search_engine_.Hamm();
  if ( !UpdateState() ) {
    return;
  }
  assert( GetCurrentStateId() == ViseServer::STATE_HAMM );

  search_engine_.Index();
  if ( !UpdateState() ) {
    return;
  }
  assert( GetCurrentStateId() == ViseServer::STATE_INDEX );

  SendCommand("_state update_now");
}

//
// methods that prepare UI data to be sent to the HTTP client
//
void ViseServer::GenerateViseIndexHtml() {
  std::ostringstream s;
  s << "<div id=\"create_engine_panel\">";
  s << "<input id=\"vise_search_engine_name\" name=\"vise_search_engine_name\" value=\"ballads\" onclick=\"document.getElementById('vise_search_engine_name').value=''\" size=\"20\" autocomplete=\"off\">";
  s << "<div class=\"action_button\" onclick=\"_vise_create_search_engine()\">&nbsp;&nbsp;Create</div>";
  s << "</div>";
  s << "<div id=\"load_engine_panel\">";

  // iterate through all directories in vise_enginedir_
  boost::filesystem::directory_iterator dir_it( vise_enginedir_ ), end_it;
  while ( dir_it != end_it ) {
    boost::filesystem::path p = dir_it->path();
    if ( boost::filesystem::is_directory( p ) ) {
      std::string name = p.filename().string();
      s << "<a onclick=\"_vise_load_search_engine('" << name << "')\" title=\"ox5k\">"
        << "<figure></figure>"
        << "<p>" << name << "</p>"
        << "</a>";
    }
    ++dir_it;
  }
  s << "</div>";
  vise_index_html_ = s.str();
  vise_index_html_reload_ = false;
}

std::string ViseServer::GetStateJsonData() {
  std::ostringstream json;
  json << "{ \"id\" : \"search_engine_state\",";
  json << "\"state_id_list\" : [ 0";
  for ( unsigned int i=1 ; i < state_id_list_.size(); i++ ) {
    json << "," << i;
  }
  json << "], \"state_name_list\" : [ \"" << state_name_list_.at(0) << "\"";
  for ( unsigned int i=1 ; i < state_id_list_.size(); i++ ) {
    json << ", \"" << state_name_list_.at(i) << "\"";
  }
  json << "], \"state_info_list\" : [ \"" << state_info_list_.at(0) << "\"";
  for ( unsigned int i=1 ; i < state_id_list_.size(); i++ ) {
    json << ", \"" << state_info_list_.at(i) << "\"";
  }
  json << "], \"current_state_id\" : " << state_id_ << ", ";
  json << "\"search_engine_name\" : \"" << search_engine_.GetName() << "\" }";

  return json.str();
}

//
// helper functions
//

// extract parts of the HTTP get resource name uri
// for example:
//   GET /Query?docId=526&x0=40&y0=20&x1=300&y1=400&results=20
// state_name = ox5k
// sta
//
void ViseServer::ParseHttpMethodUri(const std::string http_method_uri,
                                    std::string &resource_name,
                                    std::map< std::string, std::string > &resource_args) {
  if ( http_method_uri == "" ) {
    return;
  }

  std::vector< std::string > tokens;
  SplitString( http_method_uri, '?', tokens );
  // assert( tokens.size() == 2 );

  resource_name = tokens.at(0);
  if ( resource_name.at(0) == '/' ) {
    resource_name = resource_name.substr( 1, std::string::npos ); // remove prefix "/"
  }

  if ( tokens.size() == 1 ) {
    return;
  }

  for ( unsigned int i=1; i<tokens.size(); i++ ) {
    std::vector< std::string > argi;
    SplitString( tokens.at(i), '=', argi );
    // assert( argi.size() == 2 );

    std::string key   = argi.at(0);
    std::string value = argi.at(1);
    resource_args.insert( std::make_pair<std::string, std::string>(key, value) );
  }
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

int ViseServer::LoadFile(const std::string filename, std::string &file_contents) {
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

void ViseServer::SplitString( const std::string s, char sep, std::vector<std::string> &tokens ) {
  if ( s.length() == 0 ) {
    return;
  }

  unsigned int start = 0;
  for ( unsigned int i=0; i<s.length(); ++i) {
    if ( s.at(i) == sep ) {
      tokens.push_back( s.substr(start, (i-start)) );
      start = i + 1;
    }
  }
  tokens.push_back( s.substr(start, s.length()) );
}

bool ViseServer::StringStartsWith( const std::string &s, const std::string &prefix ) {
  if ( s.substr(0, prefix.length()) == prefix ) {
    return true;
  } else {
    return false;
  }
}

std::string ViseServer::GetHttpContentType( boost::filesystem::path fn) {
  std::string ext = fn.extension().string();
  std::string http_content_type = "unknown";
  if ( ext == ".jpg" ) {
    http_content_type = "image/jpeg";
  } else if ( ext == ".png" ) {
    http_content_type = "image/png";
  }
  return http_content_type;
}

//
// logging statistics
//
void ViseServer::AddTrainingStat(std::string dataset_name, std::string state_name, unsigned long time_sec, unsigned long space_bytes) {
  std::time_t t = std::time(NULL);
  char date_str[100];
  std::strftime(date_str, sizeof(date_str), "%F,%T", std::gmtime(&t));

  training_stat_f << "\n" << date_str  << "," << dataset_name << "," << state_name << "," << time_sec << "," << space_bytes << std::flush;
}
