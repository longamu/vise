#include "ViseServer.h"

const int ViseServer::STATE_NOT_LOADED;
const int ViseServer::STATE_INIT;
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

  state_id_list_.push_back( ViseServer::STATE_INIT );
  state_name_list_.push_back( "Initialize" );
  state_html_fn_list_.push_back( "Initialize.html" );
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
  state_name_list_.push_back( "Descriotor" );
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
  std::cout << "\nRequest = " << http_request << std::flush;
  std::cout << "\nMethod = [" << http_method << "]" << std::flush;
  std::cout << "\nResource = [" << http_method_uri << "]" << std::flush;

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

    std::string resource_name = http_method_uri.substr(1, std::string::npos); // remove prefix "/"
    int state_id = GetStateId( resource_name );
    if ( state_id != -1 ) {
      ServeStateHtmlPage( state_id, p_socket );
      p_socket->close();
      return;
    }
    // otherwise, say not found
    SendHttpNotFound( p_socket );
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
            SendControl("VISE_STATE_HAS_CHANGED");
          }
          p_socket->close();
          return;
          // send control message to set loaded engine name
        } else if ( tokens.at(0) == "load_search_engine" ) {
          // @todo: load search engine to last saved state
        } else {
          // unknown command
          SendHttpNotFound( p_socket );
          p_socket->close();
          return;
        }
      } else {
        // unexpected POST data
        SendHttpNotFound( p_socket );
        p_socket->close();
        return;
      }
    } else {
      // this post data corresponds to a search engine
      // handle this post data
    }
  }

    /*
    std::vector<std::string> tokens;
    SplitString( http_method_uri, '/', tokens );

    if ( tokens.size() > 1 && tokens.at(1).length() != 0 ) {
      std::string requested_engine_name = tokens.at(1);
      if ( search_engine_.GetName() != requested_engine_name ) {
        InitSearchEngine( requested_engine_name );
        UpdateState();

        // redirect user to the page that allows initialization of this search engine
        SendHttpResponse( vise_main_html_, p_socket );
        p_socket->close();
        return;
      }

      std::string resource;
      if ( tokens.size() == 2 ) {
        // http://localhost:8080/engine_name
        resource = GetCurrentStateName();
      } else if ( tokens.size() == 3 ) {
        // http://localhost:8080/engine_name/StateName
        resource = tokens.at(2);
      } else {
        SendHttpNotFound( p_socket );
      }

      HandleGetRequest( resource, p_socket);
      p_socket->close();
    } else {
      // we did not recognize the entered uri
      SendHttpNotFound( p_socket );
    }
    p_socket->close();
    return;
  }
    */
  /*
  // debug
  for ( unsigned int i=0; i<tokens.size(); i++ ) {
    std::cout << "\ntokens[" << i << "] = " << tokens.at(i) << std::flush;
  }

  std::cout << "\nhttp_method_uri = " << http_method_uri << std::flush;
  std::cout << "\nmsg_uri_ = " << msg_url_ << std::flush;
  */

  /*
  if ( http_method == "GET " ) { // note the extra space in "GET "
    // check if this is a request for message
    if ( tokens.size() == 3 ) {
      if ( tokens.at(2) == "_message" ) {
        std::string msg;
        vise_message_queue_.BlockingPop( msg );
        SendRawResponse( msg, p_socket );
        //std::cout << "\nSent message: " << msg << std::flush;
        return;
      }
    }

    if ( tokens.at(0) == "" && tokens.at(1).length() != 0 ) {
      if ( tokens.size() < 3 ) {
        if ( tokens.at(1) == "favicon.ico" ) {
          // @todo: implement this in the future
          SendHttpNotFound( p_socket );
        } else {
          // http://localhost:8080/search_engine_name
          search_engine_name = tokens.at(1);
          if ( search_engine_name == search_engine_.Name() ) {
            SendHttpResponse(html_vise_main_, p_socket);
          } else {
            InitializeSearchEngine(search_engine_name);
            SendMessage("Initialized search engine " + search_engine_name);
            search_engine_.MoveToNextState();
          }
        }
      } else {
        std::string resource = tokens.at(2);
        HandleGetRequest(resource, p_socket);
      }
    } else {
      SendHttpNotFound( p_socket );
    }
  } else if ( http_method == "POST" ) {
    if ( tokens.at(0) == "" && tokens.at(1).length() != 0 ) {
      if ( tokens.size() < 3 ) {
        SendHttpNotFound( p_socket );
      } else {
        // http://localhost:8080/search_engine_name
        search_engine_name = tokens.at(1);
        if ( search_engine_name == search_engine_.Name() ) {
          std::string resource = tokens.at(2);
          std::string http_post_data;
          ExtractHttpContent(http_request, http_post_data);

          HandlePostRequest( search_engine_name, resource, http_post_data, p_socket);
        } else {
          SendHttpNotFound( p_socket );
        }
      }
    }
  } else {
    std::cerr << "\nUnknown http_method : " << http_method << std::flush;
    SendHttpNotFound( p_socket );
  }
  p_socket->close();
  */
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

void ViseServer::SendMessage(std::string message) {
  SendPacket( "message", message);
}

void ViseServer::SendStatus(std::string status) {
  SendPacket( "status", status );
}

void ViseServer::SendControl(std::string control) {
  SendPacket("control", control );
}

void ViseServer::SendPacket(std::string type, std::string message) {
  std::ostringstream s;
  s << GetCurrentStateName() << " " << type << " " << message;
  vise_message_queue_.Push( s.str() );
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

int ViseServer::LoadFile(std::string filename, std::string &file_contents) {
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

void ViseServer::SplitString( std::string s, char sep, std::vector<std::string> &tokens ) {
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

//
// Query engine state
//
void ViseServer::HandleGetRequest( std::string resource_name,
                                   boost::shared_ptr<tcp::socket> p_socket)
{
  std::cout << "\nHandleRequest() : " << resource_name << std::flush;

  if ( resource_name == "state_list" ) {

  } else {
    int state_id = GetStateId( resource_name );
    if ( state_id >= 0 ) {
      // serve the state html page
      SendHttpResponse( state_html_list_.at(state_id), p_socket );
    } else {
      SendHttpNotFound( p_socket );
    }
  }
}

//
// Update engine state
//
void ViseServer::HandlePostRequest( std::string search_engine_name,
                                    std::string resource_name,
                                    std::string post_data,
                                    boost::shared_ptr<tcp::socket> p_socket)
{
  std::cout << "\nReceived post data : resource=" << resource_name
            << ", post_data=" << post_data << std::flush;

    if ( resource_name == "Initialize" ) {
      if ( post_data == "initialize_search_engine" ) {
        if ( UpdateState() ) {
        } else {
          SendMessage("Failed to initialize search engine!");
        }
      }
    }

    /*
    else if ( resource_name == "Settings" ) {
      search_engine_.SetEngineConfig(post_data);
      SendMessage("Saved settings for  " + search_engine_name);

      // create overview
      state_html_list_.at(SearchEngine::OVERVIEW) = search_engine_.GetEngineOverview();

      search_engine_.MoveToNextState();
    } else if ( resource_name == "Overview" ) {
      if ( post_data == "proceed" ) {
        search_engine_.MoveToNextState();
      } else {
        search_engine_.MoveToPrevState();
      }
    } else if ( resource_name == "Preprocessing" ) {
      if ( post_data == "start" ) {
        search_engine_.Preprocess();
      } else if ( post_data == "proceed" ) {
        search_engine_.MoveToNextState();
      } else {
        search_engine_.MoveToPrevState();
      }
    } else if ( resource_name == "Descriptor" ) {
      if ( post_data == "start" ) {
        search_engine_.Descriptor();
      } else if ( post_data == "proceed" ) {
        search_engine_.MoveToNextState();
      } else {
        search_engine_.MoveToPrevState();
      }
    } else if ( resource_name == "Cluster" ) {
      if ( post_data == "start" ) {
        search_engine_.Cluster();
      } else if ( post_data == "proceed" ) {
        search_engine_.MoveToNextState();
      } else {
        search_engine_.MoveToPrevState();
      }
    } else if ( resource_name == "Assignment" ) {
      if ( post_data == "start" ) {
        search_engine_.Assignment();
      } else if ( post_data == "proceed" ) {
        search_engine_.MoveToNextState();
      } else {
        search_engine_.MoveToPrevState();
      }
    } else if ( resource_name == "Hamm" ) {
      if ( post_data == "start" ) {
        search_engine_.Hamm();
      } else if ( post_data == "proceed" ) {
        search_engine_.MoveToNextState();
      } else {
        search_engine_.MoveToPrevState();
      }
    } else if ( resource_name == "Index" ) {
      if ( post_data == "start" ) {
        search_engine_.Index();
      } else if ( post_data == "proceed" ) {
        search_engine_.MoveToNextState();
      } else {
        search_engine_.MoveToPrevState();
      }
    } else if ( resource_name == "Query" ) {
    }

    // ask the client to refresh its page
    SendJsonResponse( search_engine_.GetEngineStateList(), p_socket );

  } else {
    SendMessage( search_engine_.GetEngineCurrentStateName(),
                 "Unknown resource name for HTTP POST method : " + resource_name );
  }
    */
}

//
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

bool ViseServer::UpdateState() {
  if ( state_id_ == ViseServer::STATE_NOT_LOADED ) {
    // check if search engine was initialized properly
    if ( search_engine_.GetName() != "" ) {
      state_id_ = ViseServer::STATE_INIT;
      return true;
    } else {
      return false;
    }
  }
  return false;
}

void ViseServer::ServeStateHtmlPage( int state_id, boost::shared_ptr<tcp::socket> p_socket ) {
  SendHttpResponse( state_html_list_.at(state_id), p_socket );
}

//
// methods that prepare UI data to be sent to the HTTP client
//
void ViseServer::GenerateViseIndexHtml() {
  std::ostringstream s;
  s << "<div id=\"create_engine_panel\">";
  s << "<input id=\"vise_search_engine_name\" name=\"vise_search_engine_name\" value=\"ox5k\" onclick=\"document.getElementById('vise_search_engine_name').value=''\" size=\"20\" autocomplete=\"off\">";
  s << "<div class=\"action_button\" onclick=\"_vise_create_search_engine()\">&nbsp;&nbsp;Create</div>";
  s << "</div>";
  s << "<div id=\"load_engine_panel\">@todo generate a list of saved search engines</div>";
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
