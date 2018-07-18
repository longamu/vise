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

    virtual bool query_using_upload_region() = 0;
    virtual bool query_using_file_region(uint32_t file_id,
                                         unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                                         uint32_t from, uint32_t to,
                                         double score_threshold) = 0;

    virtual bool index()            = 0;
  };
}
#endif
