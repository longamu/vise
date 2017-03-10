/**
 * Module for efficiently (and generically) building inverted indices.
 */
#ifndef __IIDX_BUILDER_HPP
#define __IIDX_BUILDER_HPP

// Files bigger than 2GB
#define _FILE_OFFSET_BITS 64
// -

#include <stdint.h>
#include <cstdio>

// Relja: don't think this is needed (taken from James):
// For posix_fadvise
// #define _XOPEN_SOURCE 600

#include <fcntl.h>
// -

#include <algorithm>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <queue>
#include <vector>

#include "jp_pickle.hpp"

// E.g. for a standard inverted index:
//  KeyType := uint32_t (term_id)
//  ValueType := pair<uint32_t, uint32_t> (doc_id, tf)
template<class KeyType, class DataType>
class
iidx_builder
{
  static const uint64_t max_buffer_size = 1073741824; // 1 GB
  //static const uint64_t max_buffer_size = 107374182; // 1 GB
  typedef std::pair<KeyType,DataType> ValueType;

  // 0 - adding terms, 1 - merging terms.
  int phase_;
  // Buffer for adding terms.
  std::vector<ValueType> buffer_;
  // List of current files.
  std::list<std::string> fnames_;
  // Temp directory.
  std::string temp_dir_;
  // Verbosity level
  int verbose_;
  // Elem size
  size_t esize_;

  // For debugging.
public:
  uint64_t bytes_read;
  uint64_t bytes_written;

private:
  std::string
  get_temp_fn()
  {
    // TODO: change to safe
    char *name;
    if (temp_dir_ != "")
        name= tempnam(temp_dir_.c_str(),NULL);
    else
        name= tempnam(NULL,NULL);
    std::string res= name;
    free(name);
    return res;
  }

  void
  save_buffer()
  {
    // 1. Sort the buffer first of all.
    std::sort(buffer_.begin(), buffer_.end());

    // 2. Save it to a temporary file.
    std::string temp_fn = get_temp_fn();

    FILE *fobj = fopen(temp_fn.c_str(), "wb");
    if (!fobj) {
      throw std::runtime_error("Can't open the temp file");
    }
    // File wrapper.
    jp_pickle_file_wrap wrap(fobj);

    size_t nwritten = 0;
    while (nwritten < buffer_.size()) {
      try {
        jp_pickle_save(wrap, buffer_[nwritten]);
      } catch (const std::runtime_error& e) {
        break;
      }

      ++nwritten;
    }

    if (nwritten != buffer_.size()) {
      throw std::runtime_error("Can't write to file -- is the disk full?");
    }
    fclose(fobj);

    // 3. Add the name of this file for future.
    fnames_.push_back(temp_fn);

    bytes_written += wrap.nwritten_;

    // 4. Clear the buffer
    //buffer_.clear();
    std::vector<ValueType> newbuffer;
    buffer_.swap(newbuffer);
    buffer_.reserve(nwritten);

    if (verbose_) {
      printf("Saved %d elements to file %s.\n", (int)nwritten, temp_fn.c_str());
    }
  }

  /**
   * k-way merge of the data.
   */
  void
  kway_merge(size_t k)
  {
    if (fnames_.size() > 1) {
      std::vector<FILE*> fins;
      std::vector<jp_pickle_file_wrap> wrapins;
      std::vector<std::string> fins_names;

      typedef std::priority_queue<
                std::pair<ValueType, size_t>,
                std::vector< std::pair<ValueType, size_t> >,
                std::greater< std::pair<ValueType, size_t> > > PriQueue;

      PriQueue heap;

      size_t fnames_sz = fnames_.size();

      for (size_t i=0; i<std::min(k,fnames_sz); ++i) {
        std::string fn = fnames_.front();
        fnames_.pop_front();
        FILE* fobj = fopen(fn.c_str(),"rb");
        fins.push_back(fobj);
        fins_names.push_back(fn);
        jp_pickle_file_wrap wrap(fobj);
        wrapins.push_back(wrap);
        int fd = fileno(fobj);
        // The FADV_SEQUENTIAL should give us better read by seeking ahead.
        int errc = posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
        if (errc != 0) {
          fprintf(stderr, "posix_fadvise failed with err %d?\n", errc);
          perror("");
        }
        // Load up one element into the heap.
        try {
          ValueType obj;
          jp_pickle_load(wrap, obj);
          heap.push(std::make_pair(obj, i));
        }
        catch (std::runtime_error& e)
        {  }
      }

      if (verbose_) {
        printf("Merging %d files.\n", (int)wrapins.size());
      }
      
      // Output file.
      std::string fno = get_temp_fn();
      FILE *fo = fopen(fno.c_str(), "wb");
      jp_pickle_file_wrap wrapo(fo);

      while (heap.size()) {
        const std::pair< ValueType, size_t >& t = heap.top();
        jp_pickle_save(wrapo, t.first);
        
        try {
          ValueType obj;
          jp_pickle_load(wrapins[t.second], obj);

          heap.push( std::make_pair(obj, t.second) );
        }
        catch (std::runtime_error& e)
        { }
        
        heap.pop();
      }

      // All done.
      fclose(fo);

      bytes_written += wrapo.nwritten_;

      for (size_t i=0; i<fins.size(); ++i) {
        fclose(fins[i]);
        bytes_read += wrapins[i].nread_;
        remove(fins_names[i].c_str());
      }

      // Fix up fnames_
      fnames_.push_back(fno);
    }
  }

public:
  iidx_builder(const std::string &temp_dir, int verbose = 0)
   : phase_(0), temp_dir_(temp_dir), verbose_(verbose), esize_(0)
  { }

  void
  add_term(const KeyType& key, const DataType& data)
  {
    if (phase_ != 0) throw std::runtime_error("Can't add terms unless in phase 0");
    
    ValueType o(key, data);
    if (!esize_) {
      // Record the size.
      jp_pickle_byte_write bw;

      jp_pickle_save(bw, o);
      
      esize_ = bw.size();
      if (verbose_)
        printf("esize = %d\n", (int)esize_);

      // This rounds up to the nearest multiple of esize_.
      buffer_.reserve((max_buffer_size + esize_ - 1) / esize_);
    }
    buffer_.push_back(o);

    if (buffer_.size()*esize_ > max_buffer_size)
      save_buffer();
  }

  void
  merge_till_one()
  {
    // Switch into a different mode.
    phase_ = 1;
    
    // We might have some left over terms.
    if (buffer_.size()) save_buffer();

    while (fnames_.size()>1)
      kway_merge(64);
  }

  char*
  get_last_fn()
  {
    if (fnames_.size() != 1) throw std::runtime_error("more than one file in the list");

    return const_cast<char*>(fnames_.front().c_str());
  }
};

#endif
