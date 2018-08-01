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
    virtual bool load()             = 0;
    virtual bool unload()           = 0;

    virtual bool is_load_possible() = 0;
    virtual bool is_loaded()        = 0;

    virtual bool query_using_upload_region() = 0;
    virtual bool query_using_file_region(unsigned int file_id,
                                         unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                                         std::vector<unsigned int> &result_file_id,
                                         std::vector<std::string> &result_filename,
                                         std::vector<std::string> &result_metadata,
                                         std::vector<float> &result_score,
                                         std::vector< std::array<double, 9> > &result_H) = 0;

    virtual bool index()            = 0;

    virtual void get_filelist(const uint32_t from, const uint32_t result_count,
                              std::vector<uint32_t> &file_id_list,
                              std::vector<std::string> &filename_list) = 0;
    virtual void get_filelist(const std::string filename_regex,
                              const unsigned int from, const unsigned int result_count,
                              std::vector<uint32_t> &file_id_list,
                              std::vector<std::string> &filename_list) = 0;

    virtual uint32_t get_filelist_size() = 0;

    virtual bool file_exists(std::string filename) = 0;
    virtual bool file_exists(unsigned int file_id) = 0;
    virtual std::string get_filename_absolute_path(std::string filename) = 0;
    virtual std::string get_filename_absolute_path(unsigned int file_id) = 0;
    virtual std::string get_filename(unsigned int file_id)               = 0;

    static std::string get_search_engine_id(std::string name, std::string version) {
      return name + "/" + version;
    }

  };
}
#endif
