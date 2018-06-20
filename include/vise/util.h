/** @file   vise_request_handler.h
 *  @brief  A singleton class which handles http requests related to vise
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   20 June 2018
 */

#ifndef _VISE_UTIL_H_
#define _VISE_UTIL_H_

#include <iostream>
#include <sstream>
#include <fstream>

#define BOOST_LOG_DYN_LINK 1
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

namespace vise {
namespace util {

//
// string util
//
bool starts_with(const std::string &s, const std::string prefix);
bool ends_with(const std::string &s, const std::string suffix);
bool contains(const std::string &s, const std::string substr);

//
// file i/o
//
bool load_file_content(const boost::filesystem::path fn, std::string& file_content);

//
// http uri parsing util
//
bool has_special_char(const std::string &s);

} // end of namespace util
} // end of namespace vise

#endif
