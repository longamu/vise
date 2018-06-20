/** @file   vise_request_handler.h
 *  @brief  A singleton class which handles http requests related to vise
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   20 June 2018
 */

#include "vise/util.h"

//
// string util
//
bool vise::util::starts_with(const std::string &s, const std::string prefix) {
  if ( s.substr(0, prefix.length()) == prefix ) {
    return true;
  } else {
    return false;
  }
}

bool vise::util::ends_with(const std::string &s, const std::string suffix) {
  size_t pos = s.length() - suffix.length();
  if ( s.substr( pos ) == suffix ) {
    return true;
  } else {
    return false;
  }
}

bool vise::util::contains(const std::string &s, const std::string substr) {
  if ( s.find(substr) == std::string::npos ) {
    return false;
  } else {
    return true;
  }
}

//
// file i/o
//
bool vise::util::load_file_content(const boost::filesystem::path fn, std::string& file_content) {
  if( !boost::filesystem::exists(fn) ) {
    BOOST_LOG_TRIVIAL(debug) << "file does not exist [" << fn.string() << "]";
    return false;
  }
  if( !boost::filesystem::is_regular_file(fn) ) {
    BOOST_LOG_TRIVIAL(debug) << "not a regular file [" << fn.string() << "]";
    return false;
  }

  try {
    std::ifstream f;
    f.open(fn.string().c_str(), std::ios::in | std::ios::binary);
    f.seekg(0, std::ios::end);
    file_content.clear();
    file_content.reserve( f.tellg() );
    f.seekg(0, std::ios::beg);
    file_content.assign( (std::istreambuf_iterator<char>(f)),
                          std::istreambuf_iterator<char>() );
    f.close();
    return true;
  } catch(std::exception &e) {
    BOOST_LOG_TRIVIAL(error) << "failed to load file [" << fn.string() << "]";
    return false;
  }
}

//
// http uri parsing util
//


bool vise::util::has_special_char(const std::string &s) {
  if ( s.find('!') != std::string::npos )
    return true;
  if ( s.find('"') != std::string::npos )
    return true;
  if ( s.find('$') != std::string::npos )
    return true;
  if ( s.find('%') != std::string::npos )
    return true;
  if ( s.find('^') != std::string::npos )
    return true;
  if ( s.find('&') != std::string::npos )
    return true;
  if ( s.find('*') != std::string::npos )
    return true;
  if ( s.find('!') != std::string::npos )
    return true;
  if ( s.find('@') != std::string::npos )
    return true;
  if ( s.find('~') != std::string::npos )
    return true;
  if ( s.find('?') != std::string::npos )
    return true;
  if ( s.find("../") != std::string::npos )
    return true;
  if ( s.find("./") != std::string::npos )
    return true;
  if ( s.find(".\\") != std::string::npos )
    return true;

  /*  if ( s.find('/') != std::string::npos )
    return true;
  if ( s.find('\\') != std::string::npos )
    return true;
  */

  return false;
}

