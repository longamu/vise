/** @file   search_engine.h
 *  @brief  Abstract class representing a visual search engine
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   16 July 2018
 */

#ifndef _VISE_SEARCH_ENGINE_H_
#define _VISE_SEARCH_ENGINE_H_

#include <string>

// for filesystem i/o
#include <boost/filesystem.hpp>

using namespace std;

namespace vise {
  class search_engine {
  public:
    virtual string id()             = 0;
    virtual bool init()             = 0;  
    virtual bool is_load_possible() = 0;
    virtual bool load()             = 0;
    virtual bool unload()           = 0;
    virtual bool query()            = 0;
    virtual bool index()            = 0;
  };
}
#endif
