/**
 * Very generic inverted index (can have different key and value).
 */
#ifndef __IIDX_GENERIC_HPP
#define __IIDX_GENERIC_HPP

#define _FILE_OFFSET_BITS 64 // Files bigger than 2GB.

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "jp_pickle.hpp"

template<class KeyType, class DataType>
class
iidx_generic
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

private:
  void
  save_structures() const
  {
    uint32_t magic_num = 0x0f0f0f00;
    uint64_t off = ftello(fobj_);

    jp_pickle_file_wrap wrapper(fobj_);

    jp_pickle_save(wrapper, kmap_);
    jp_pickle_save(wrapper, nterms_);
    jp_pickle_save(wrapper, max_key_);
    jp_pickle_save(wrapper, max_data_);

    jp_pickle_save(wrapper, off);
    jp_pickle_save(wrapper, magic_num);
  }

  void
  load_structures()
  {
    fseek(fobj_, -12, SEEK_END);

    uint64_t off;
    uint32_t magic_num = 0;
    
    jp_pickle_file_wrap wrapper(fobj_);

    jp_pickle_load(wrapper, off);
    jp_pickle_load(wrapper, magic_num);

    if (magic_num != 0x0f0f0f00) {
      throw std::runtime_error("magic number doesn't match");
    }
    
    wrapper.fseeko_(off);

    jp_pickle_load(wrapper, kmap_);
    jp_pickle_load(wrapper, nterms_);
    jp_pickle_load(wrapper, max_key_);
    jp_pickle_load(wrapper, max_data_);
  }

public:
  iidx_generic(const std::string& fn, char mode='r')
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

  ~iidx_generic()
  {
    if (mode_ != 'r')
      save_structures();
    fclose(fobj_);
    fobj_ = 0;
  }

  void
  load_from_fn(const std::string& fn)
  {
    nterms_= 0;
    FILE* fin = fopen(fn.c_str(), "rb");
    jp_pickle_file_wrap br(fin);
    jp_pickle_file_wrap bw(fobj_);
    
    std::vector<DataType> data;

    KeyType cur_key;
    ValueType term;
    bool feof_fin= false;
    // Read the first term.
    jp_pickle_load(br, term); cur_key = term.first;
    while (!feof_fin) {
      if (term.first > max_key_) max_key_ = term.first;

      data.clear();
      while (!feof_fin && cur_key == term.first) {
        if (term.second > max_data_) max_data_ = term.second;

        data.push_back(term.second);

        try { // If this throws, then feof will be set.
          jp_pickle_load(br, term);
        } catch (const std::runtime_error& e) { feof_fin= true; break; }
      }
      
      // We should write whatever's in data.
      uint64_t off = ftello(fobj_);
      uint32_t sz = (uint32_t)data.size();

      jp_pickle_save(bw, data);
      
      nterms_ += data.size();
      kmap_[cur_key] = std::pair<uint64_t, uint32_t>(off, sz);

      // Set the cur_key to the new term.
      cur_key = term.first;
    }
    fclose(fin);
  }

  void
  word_terms(const KeyType &key, std::vector< DataType > &terms) const
  {
    if (!kmap_.count(key)) {
      throw std::runtime_error("word not found");
    }

    std::pair<uint64_t, uint32_t> off_sz_pair = kmap_.find(key)->second;
    
    jp_pickle_file_wrap wrapper(fobj_, &(off_sz_pair.first) );

    jp_pickle_load(wrapper, terms);
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

template<class Iter>
void
accumulate_histogram(Iter beg, Iter end, unsigned* sdescr, unsigned ndims, float* doc_itsz_hist, float qu_weight)
{
  unsigned i, j, itsz, did;
  const unsigned* tsdescr;
  while (beg != end) {
    i = 0;
    j = 0;
    itsz = 0;

    did = beg->first;
    tsdescr = &beg->second[0];

    while (i < ndims && j < ndims) {
      if (sdescr[i] < tsdescr[j]) ++i;
      else if (sdescr[i] > tsdescr[j]) ++j;
      else {
        ++i;
        ++j;
        ++itsz;
      }
    }

    if (itsz < ndims) {
      doc_itsz_hist[did*ndims + itsz] += qu_weight;
    }

    ++beg;
  }
}

#endif
