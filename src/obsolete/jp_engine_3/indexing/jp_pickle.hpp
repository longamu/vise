#ifndef __JP_PICKLE_HPP
#define __JP_PICKLE_HPP

//#include <boost/type_traits/is_pod.hpp>
#include <stdint.h>

#include <stdexcept>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>

// --- POD TYPES ---
template<class T> struct jp_pickle_is_pod { static const bool value = false; };
template<> struct jp_pickle_is_pod<char> { static const bool value = true; };
template<> struct jp_pickle_is_pod<unsigned char> { static const bool value = true; };
template<> struct jp_pickle_is_pod<short> { static const bool value = true; };
template<> struct jp_pickle_is_pod<unsigned short> { static const bool value = true; };
template<> struct jp_pickle_is_pod<int> { static const bool value = true; };
template<> struct jp_pickle_is_pod<unsigned int> { static const bool value = true; };
template<> struct jp_pickle_is_pod<int64_t> { static const bool value = true; };
template<> struct jp_pickle_is_pod<uint64_t> { static const bool value = true; };
// --- ---

class
jp_pickle_file_wrap
{
  FILE *fobj_p;
  int fobj_d;
  uint64_t off_;

public:
  uint64_t nwritten_;
  uint64_t nread_;

public:
  jp_pickle_file_wrap(FILE* fobj, uint64_t *foff= NULL)
    : fobj_p(fobj), nwritten_(0), nread_(0)
  {
      fobj_d= fileno(fobj);
      if (foff==NULL)
          off_= ftello(fobj);
      else
          off_= *foff;
  }

  void
  read(char* out, uint32_t size)
  {
//     size_t nmemb = fread((void*)out, size, 1, fobj_);
    size_t nmemb= pread64(fobj_d, (void*)out, size, off_);
    if (nmemb!=size) {
      throw std::runtime_error("pread64 returned 0!");
    }
    nread_ += size;
    off_+= size;
  }
  
  void
  write(const char *data, uint32_t size)
  {
    size_t nmemb = fwrite((void*)data, size, 1, fobj_p);
    if (nmemb!=1) {
      throw std::runtime_error("fwrite returned 0!");
    }
    nwritten_ += size;
  }
  
  void
      fseeko_( uint64_t off ){ off_= off; }
  
  uint64_t
      ftello_(){ return off_; }
};

class
jp_pickle_byte_read
{
  char *ptr_;

  const uint32_t size_;
  uint32_t ind_;

public:

  jp_pickle_byte_read(uint32_t sz)
   : size_(sz), ind_(0)
  {
    ptr_ = (char*)malloc(sz);
  }

  jp_pickle_byte_read(char *ptr, uint32_t sz)
   : ptr_(ptr), size_(sz), ind_(0)
  { }

  ~jp_pickle_byte_read()
  { if (ptr_) free(ptr_); }

  void
  read(char *out, uint32_t size)
  {
    memcpy(out, &ptr_[ind_], size);
    ind_ += size;
  }

  char*
  get_raw()
  { return ptr_; }
};

class
jp_pickle_byte_write
{
  char *ptr_;
  
  uint32_t size_;
  uint32_t max_size_;

  void
  grow(uint32_t sz)
  {
    if (sz < max_size_) return;

    while (max_size_<sz) max_size_*=2;

    char *ptr = (char*)malloc(max_size_);
    
    memcpy(ptr, ptr_, size_);
    free(ptr_);
    ptr_ = ptr;
  }
public:
  jp_pickle_byte_write()
  {
    max_size_ = 4096;
    size_ = 0;
    ptr_ = (char*)malloc(max_size_);
  }

  ~jp_pickle_byte_write()
  { free(ptr_); }

  void
  write(const char *data, uint32_t size)
  {
    grow(size_+size);

    memcpy(&ptr_[size_], data, size);

    size_ += size;
  }

  uint32_t
  size()
  { return size_; }

  const char*
  raw() const
  { return ptr_; }
};

#define JP_PICKLE_SIMPLE_PICKLE_SAVE(type) \
  template<class BW> \
  inline \
  void \
  jp_pickle_save(BW &data, const type &t) \
  { \
    data.write((const char*)&t, sizeof(type)); \
  }

#define JP_PICKLE_SIMPLE_PICKLE_LOAD(type) \
  template<class BR> \
  inline \
  void \
  jp_pickle_load(BR &data, type &t) \
  { \
    data.read((char*)&t, sizeof(type)); \
  }

JP_PICKLE_SIMPLE_PICKLE_LOAD(char)
JP_PICKLE_SIMPLE_PICKLE_LOAD(int8_t)
JP_PICKLE_SIMPLE_PICKLE_LOAD(uint8_t)
JP_PICKLE_SIMPLE_PICKLE_LOAD(int16_t)
JP_PICKLE_SIMPLE_PICKLE_LOAD(uint16_t)
JP_PICKLE_SIMPLE_PICKLE_LOAD(int32_t)
JP_PICKLE_SIMPLE_PICKLE_LOAD(uint32_t)
JP_PICKLE_SIMPLE_PICKLE_LOAD(int64_t)
JP_PICKLE_SIMPLE_PICKLE_LOAD(uint64_t)
JP_PICKLE_SIMPLE_PICKLE_LOAD(float)
JP_PICKLE_SIMPLE_PICKLE_LOAD(double)

JP_PICKLE_SIMPLE_PICKLE_SAVE(char)
JP_PICKLE_SIMPLE_PICKLE_SAVE(int8_t)
JP_PICKLE_SIMPLE_PICKLE_SAVE(uint8_t)
JP_PICKLE_SIMPLE_PICKLE_SAVE(int16_t)
JP_PICKLE_SIMPLE_PICKLE_SAVE(uint16_t)
JP_PICKLE_SIMPLE_PICKLE_SAVE(int32_t)
JP_PICKLE_SIMPLE_PICKLE_SAVE(uint32_t)
JP_PICKLE_SIMPLE_PICKLE_SAVE(int64_t)
JP_PICKLE_SIMPLE_PICKLE_SAVE(uint64_t)
JP_PICKLE_SIMPLE_PICKLE_SAVE(float)
JP_PICKLE_SIMPLE_PICKLE_SAVE(double)

#if defined(_GLIBCXX_STRING) || defined(_CPP_STRING)
template<class BW>
inline
void
jp_pickle_save(BW &data, const std::string &s)
{
  uint32_t sz = (uint32_t)s.size();
  jp_pickle_save(data, sz);

  for (uint32_t i = 0; i<sz; ++i) {
    jp_pickle_save(data, s[i]);
  }
}

template<class BR>
inline
void
jp_pickle_load(BR &data, std::string &s)
{
  uint32_t sz = 0;
  jp_pickle_load(data, sz);

  s.resize(sz);
  for (uint32_t i = 0; i<sz; ++i) {
    jp_pickle_load(data, s[i]);
  }
}
#endif

template<class BW, class S, class T>
inline
void
jp_pickle_save(BW &data, const std::pair<S, T> &t)
{
  jp_pickle_save(data, t.first);
  jp_pickle_save(data, t.second);
}

template<class BR, class S, class T>
inline
void
jp_pickle_load(BR &data, std::pair<S, T> &t)
{
  jp_pickle_load(data, t.first);
  jp_pickle_load(data, t.second);
}

#if defined(_GLIBCXX_VECTOR) || defined(_CPP_VECTOR)
template<class BW, class T, class A>
inline
void
jp_pickle_save(BW &data, const std::vector<T, A> &t)
{
  uint32_t sz = (uint32_t)t.size();
  jp_pickle_save(data, sz);

  if (jp_pickle_is_pod<T>::value) {
    data.write((const char*)&t[0], sizeof(T)*sz);
  }
  else {
    for (uint32_t i=0; i<sz; ++i) {
      jp_pickle_save(data, t[i]);
    }
  }
}

template<class BR, class T, class A>
inline
void
jp_pickle_load(BR &data, std::vector<T, A> &t)
{
  uint32_t sz;
  jp_pickle_load(data, sz);

  t.resize(sz);

  if (jp_pickle_is_pod<T>::value) {
    data.read((char*)&t[0], sizeof(T)*sz);
  }
  else {
    for (size_t i=0; i<sz; ++i) {
      jp_pickle_load(data, t[i]);
    }
  }
}
#endif

#if defined(_GLIBCXX_MAP) || defined(_CPP_MAP)
template<class BW, class S, class T>
inline
void
jp_pickle_save(BW &data, const std::map<S, T> &t)
{
  uint32_t sz = (uint32_t)t.size();
  jp_pickle_save(data, sz);

  for (typename std::map<S, T>::const_iterator it = t.begin();
       it != t.end();
       ++it)
  {
    jp_pickle_save(data, it->first);
    jp_pickle_save(data, it->second);
  }
}

template<class BR, class S, class T>
inline
void
jp_pickle_load(BR &data, std::map<S, T> &m)
{
  uint32_t sz = 0;
  jp_pickle_load(data, sz);

  for (uint32_t i=0; i<sz; ++i) {
    S s;
    T t;
    jp_pickle_load(data, s);
    jp_pickle_load(data, t);
    m[s] = t;
  }
}

template<class BW, class S, class T>
inline
void
jp_pickle_save(BW &data, const std::multimap<S, T> &t)
{
  jp_pickle_save(data, (uint32_t)t.size());
  for (typename std::multimap<S, T>::const_iterator it = t.begin();
       it != t.end();
       ++it)
  {
    jp_pickle_save(data, it->first);
    jp_pickle_save(data, it->second);
  }
}

template<class BR, class S, class T>
inline
void
jp_pickle_load(BR &data, std::multimap<S, T> &m)
{
  uint32_t sz = 0;
  jp_pickle_load(data, sz);

  for (uint32_t i=0; i<sz; ++i) {
    S s;
    T t;
    jp_pickle_load(data, s);
    jp_pickle_load(data, t);
    m.insert(std::make_pair(s, t));
  }
}
#endif

#if defined(_GLIBCXX_SET) || defined(_CPP_SET)
template<class BW, class T>
inline
void
jp_pickle_save(BW &data, const std::set<T> &t)
{
  uint32_t sz = (uint32_t)t.size();
  jp_pickle_save(data, sz);

  for (typename std::set<T>::const_iterator it = t.begin();
       it != t.end();
       ++it)
  {
    jp_pickle_save(data, *it);
  }
}

template<class BR, class T>
inline
void
jp_pickle_load(BR &data, std::set<T> &s)
{
  uint32_t sz = 0;
  jp_pickle_load(data, sz);

  for (uint32_t i=0; i<sz; ++i) {
    T t;
    jp_pickle_load(data, t);
    s.insert(t);
  }
}
#endif

#endif
