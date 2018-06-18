/**
 * Very generic inverted index (can have different key and value).
 */
#ifndef __IIDX_GENERIC_NONTHREADSAFE_HPP
#define __IIDX_GENERIC_NONTHREADSAFE_HPP

#define _FILE_OFFSET_BITS 64 // Files bigger than 2GB.

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "jp_pickle_nonthreadsafe.hpp"

template<class KeyType, class DataType>
class
iidx_generic_nonthreadsafe
{
public:
  typedef std::pair<KeyType, DataType> ValueType;
  // Map from the key to (off (in bytes), sz (in nelems))
  typedef std::map< KeyType, std::pair<uint64_t, uint32_t> > MapType;
  // Iterator
  typedef typename std::vector< DataType >::const_iterator const_iterator;

private:
  FILE* fobj_;
  char mode_;
  //uint64_t fsz; // The file size of the number of terms written.

  uint64_t nterms_;

  KeyType max_key_;
  DataType max_data_;

  MapType kmap_;

  // TRANSIENT
  KeyType loaded_key_;
  std::vector< DataType > loaded_terms_;

private:
  
  void
  load_structures()
  {
    fseek(fobj_, -12, SEEK_END);

    uint64_t off;
    uint32_t magic_num = 0;

    jp_pickle_file_wrap_nonthreadsafe wrapper(fobj_);

    jp_pickle_load_nonthreadsafe(wrapper, off);
    jp_pickle_load_nonthreadsafe(wrapper, magic_num);

    if (magic_num != 0x0f0f0f00) {
      throw std::runtime_error("magic number doesn't match");
    }

    fseeko(fobj_, off, SEEK_SET);

    jp_pickle_load_nonthreadsafe(wrapper, kmap_);
    jp_pickle_load_nonthreadsafe(wrapper, nterms_);
    jp_pickle_load_nonthreadsafe(wrapper, max_key_);
    jp_pickle_load_nonthreadsafe(wrapper, max_data_);
  }

  void
  load_word(const KeyType &key)
  {
    if (!kmap_.count(key)) {
      throw std::runtime_error("word not found");
    }

    std::pair<uint64_t, uint32_t> off_sz_pair = kmap_[key];
    fseeko(fobj_, off_sz_pair.first, SEEK_SET);

    jp_pickle_file_wrap_nonthreadsafe wrapper(fobj_);

    jp_pickle_load_nonthreadsafe(wrapper, loaded_terms_);

    loaded_key_ = key;
  }

public:
  iidx_generic_nonthreadsafe(const std::string& fn, char mode='r')
  {
    fobj_ = 0;
    max_key_ = KeyType();
    max_data_ = DataType();
    mode_ = mode;
    if (mode == 'r') {
      fobj_ = fopen(fn.c_str(), "rb");

      if (!fobj_)
        throw std::runtime_error("can't open inverted index (does it exist?)");

      load_structures();
    }
    else if (mode == 'w') {
      fobj_ = fopen(fn.c_str(), "wb");

      if (!fobj_)
        throw std::runtime_error("can't open inverted index (does it exist?)");
    }
    else {
      throw std::runtime_error("mode not recognized");
    }
  }

  ~iidx_generic_nonthreadsafe()
  {
    fclose(fobj_);
    fobj_ = 0;
  }

  void
  load_from_fn(const std::string& fn)
  {
    FILE* fin = fopen(fn.c_str(), "rb");
    jp_pickle_file_wrap_nonthreadsafe br(fin);
    jp_pickle_file_wrap_nonthreadsafe bw(fobj_);
    
    std::vector<DataType> data;

    KeyType cur_key;
    ValueType term;
    // Read the first term.
    jp_pickle_load_nonthreadsafe(br, term); cur_key = term.first;
    while (!feof(fin)) {
      if (term.first > max_key_) max_key_ = term.first;

      data.clear();
      while (!feof(fin) && cur_key == term.first) {
        if (term.second > max_data_) max_data_ = term.second;

        data.push_back(term.second);

        try { // If this throws, then feof will be set.
          jp_pickle_load_nonthreadsafe(br, term);
        } catch (const std::runtime_error& e) { break; }
      }
      
      // We should write whatever's in data.
      uint64_t off = ftello(fobj_);
      uint32_t sz = (uint32_t)data.size();

      jp_pickle_save_nonthreadsafe(bw, data);
      
      nterms_ += data.size();
      kmap_[cur_key] = std::pair<uint64_t, uint32_t>(off, sz);

      // Set the cur_key to the new term.
      cur_key = term.first;
    }
  }

  const_iterator
  begin(const KeyType& k)
  {
    load_word(k);

    return loaded_terms_.begin();
  }

  const_iterator
  end(const KeyType& k)
  {
    if (k != loaded_key_) throw std::runtime_error("end() called with wrong id");
    return loaded_terms_.end();
  }

  const KeyType&
  max_key() const
  {
    return max_key_;
  }

  const DataType&
  max_data() const
  {
    return max_data_;
  }

  size_t
  nterms() const
  { return nterms_; }
};

#endif
