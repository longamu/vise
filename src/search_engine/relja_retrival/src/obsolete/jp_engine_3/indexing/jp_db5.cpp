#define _FILE_OFFSET_BITS 64

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <zlib.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include "compression/minilzo.h"

#include "jp_db5.hpp"

#pragma pack(0)
struct
entry_info_0_1
{
  uint32_t bnum_;
  uint32_t extent_;

  union {
    struct {
      unsigned is_gziped_ : 1;
      unsigned is_lzoed_ : 1;
      int unused_1_ : 30;
      int unused_2_;
    };
    uint64_t attr_;
  };
};
#pragma pack()

void
gzip_compress(char **buffer, size_t *size,
              int complevel)
{
  char *input_buffer = *buffer;
  size_t input_size = *size;

  const Bytef *z_src = (const Bytef*)input_buffer;
  Bytef       *z_dst = 0;
  uLongf       z_dst_nbytes = (uLongf)((input_size * 1.001) + 13);
  uLong        z_src_nbytes = (uLong)input_size;
  int          aggression = complevel;
  int status;

  z_dst = (Bytef*)malloc(z_dst_nbytes);

  status = compress2(z_dst, &z_dst_nbytes, z_src, z_src_nbytes, aggression);

  if (status != Z_OK) {
    //printf("%d %d\n", z_dst_nbytes, z_src_nbytes);
    throw std::runtime_error("compress2 error");
  }

  free(input_buffer);
  
  *buffer = (char*)z_dst;
  *size = z_dst_nbytes;
}

void
gzip_uncompress(char **buffer, size_t *size)
{
  char *input_buffer = *buffer;
  size_t input_size = *size;

  z_stream z_strm;
  size_t nalloc = input_size;

  void *outbuf = malloc(nalloc);
  if (outbuf==0) throw "oom error";

  memset(&z_strm, 0, sizeof(z_stream));
  z_strm.next_in = (Bytef*)input_buffer;
  z_strm.avail_in = input_size;
  z_strm.next_out = (Bytef*)outbuf;
  z_strm.avail_out = nalloc;

  if (inflateInit(&z_strm)!=Z_OK) {
    throw "inflateInit failed";
  }

  int status;
  do {
    status = inflate(&z_strm, Z_SYNC_FLUSH);

    if (status == Z_STREAM_END)
      break;

    if (status != Z_OK) {
      throw "inflate() failed";
    }
    else {
      if (z_strm.avail_out == 0) {
        void *new_outbuf;
        nalloc*=2;
        new_outbuf = realloc(outbuf, nalloc);
        if (!new_outbuf) { throw "oom error"; }
        outbuf = new_outbuf;

        z_strm.next_out = (unsigned char*)outbuf + z_strm.total_out;
        z_strm.avail_out = (uInt)(nalloc - z_strm.total_out);
      }
    }
  }
  while (status==Z_OK);

  free(input_buffer);

  *buffer = (char*)outbuf;
  *size = z_strm.total_out;
  
  (void)inflateEnd(&z_strm);
}

void
lzo_compress(char **buffer, size_t *size)
{
  char *input_buffer = *buffer;
  size_t input_size = *size;
  char *output_buffer = 0;
  lzo_uint output_size = 0;

  unsigned char *workmem = (unsigned char*)malloc(LZO1X_1_MEM_COMPRESS);

  // We'll store the original length in the first 4 bytes.
  output_buffer = (char*)malloc(input_size + input_size/16 + 64 + 3 + sizeof(uint32_t));

  int errc = lzo1x_1_compress((unsigned char*)input_buffer, input_size,
                              (unsigned char*)output_buffer + sizeof(uint32_t), &output_size,
                              workmem);
  if (errc != LZO_E_OK) {
    throw "LZO compression error!";
  }

  *((uint32_t*)output_buffer) = (uint32_t)input_size;
  
  free(input_buffer);
  free(workmem);

  *buffer = output_buffer;
  *size = output_size + sizeof(uint32_t);
}

void
lzo_uncompress(char **buffer, size_t *size)
{
  char *input_buffer = *buffer;
  size_t input_size = *size;
  lzo_uint output_size = *((uint32_t*)input_buffer);
  char *output_buffer = (char*)malloc(output_size);

  int errc = lzo1x_decompress((unsigned char*)input_buffer + sizeof(uint32_t), input_size - sizeof(uint32_t),
                              (unsigned char*)output_buffer, &output_size,
                              NULL);
  if (errc != LZO_E_OK) {
    throw "LZO decompression error!";
  }

  free(input_buffer);

  *buffer = output_buffer;
  *size = output_size;
}

class
jp_db5_imp
{
public:
  int fd_; // File descriptor
  std::string fname_; // File name
  std::string mode_; // File mode

  typedef std::map< std::string, entry_info_0_1> map_type;

  map_type key_to_info_; // Mapping from key to info
  uint32_t bsize_; // Block size
  uint32_t nblocks_; // Total number of blocks in use
};

void
load_internal_data_0_1(jp_db5_imp *p)
{
  { // Read map
    ssize_t tmpout_= 0; // Relja: to remove warning that we're ignoring outputs
    ++tmpout_; // just to avoid "warning: variable ‘tmpout_’ set but not used [-Wunused-but-set-variable]"
    uint32_t sz;
    tmpout_= read(p->fd_, &sz, sizeof(uint32_t));
    for (uint32_t i=0; i<sz; ++i) {
      // Read string.
      uint32_t strsz;
      tmpout_= read(p->fd_, &strsz, sizeof(uint32_t));
      std::string str;
      str.resize(strsz);
      tmpout_= read(p->fd_, &str[0], strsz);
      // Read info.
      entry_info_0_1 info;
      tmpout_= read(p->fd_, &info, sizeof(entry_info_0_1));

      p->key_to_info_[str] = info;
    }
  }
}

void
save_internal_data_0_1(const jp_db5_imp *p)
{
  { // Save map
    ssize_t tmpout_= 0; // Relja: to remove warning that we're ignoring outputs
    ++tmpout_; // just to avoid "warning: variable ‘tmpout_’ set but not used [-Wunused-but-set-variable]"
    uint32_t sz = (uint32_t)p->key_to_info_.size();
    tmpout_= write(p->fd_, &sz, sizeof(uint32_t));
    for (jp_db5_imp::map_type::const_iterator it = p->key_to_info_.begin(); it!= p->key_to_info_.end(); ++it)
    {
      uint32_t strsz = it->first.size();
      entry_info_0_1 info = it->second;

      tmpout_= write(p->fd_, &strsz, sizeof(uint32_t));
      tmpout_= write(p->fd_, (it->first).c_str(), strsz);

      tmpout_= write(p->fd_, &info, sizeof(entry_info_0_1));
    }
  }
}

bool
fexists(const char* fname)
{
  struct stat buf;
  int errc = stat(fname, &buf);
  
  if (errc) return false;
  else return true;
}

void
load_internal_data(jp_db5_imp* p)
{
  uint32_t version = 0, magic_num = 0;
  ssize_t tmpout_= 0; // Relja: to remove warning that we're ignoring outputs
  ++tmpout_; // just to avoid "warning: variable ‘tmpout_’ set but not used [-Wunused-but-set-variable]"

  lseek64(p->fd_, -4*(signed long)sizeof(uint32_t), SEEK_END);

  tmpout_= read(p->fd_, &p->bsize_, sizeof(uint32_t));
  tmpout_= read(p->fd_, &p->nblocks_, sizeof(uint32_t));
  tmpout_= read(p->fd_, &version, sizeof(uint32_t));
  tmpout_= read(p->fd_, &magic_num, sizeof(uint32_t));

  if (magic_num != 0xf00ff00f) {
    throw std::runtime_error("IOError: file exists but doesn't look like a db5 file");
  }

  lseek64(p->fd_, (off64_t)p->nblocks_ * p->bsize_, SEEK_SET);

  if (version == 10) { // 0.10
    load_internal_data_0_1(p);
  }
  else {
    throw std::runtime_error("RuntimeError: version not supported");
  }
}

void
save_internal_data(jp_db5_imp* p)
{
  ssize_t tmpout_= 0; // Relja: to remove warning that we're ignoring outputs
  ++tmpout_; // just to avoid "warning: variable ‘tmpout_’ set but not used [-Wunused-but-set-variable]"
  // Seek to the last block.
  lseek64(p->fd_, (off64_t)p->nblocks_ * p->bsize_, SEEK_SET);
  // Write versioned info.
  save_internal_data_0_1(p);
  // Write extra info.
  uint32_t version = 10; // 0.10
  uint32_t magic_num = 0xf00ff00f;

  tmpout_= write(p->fd_, &p->bsize_, sizeof(uint32_t));
  tmpout_= write(p->fd_, &p->nblocks_, sizeof(uint32_t));
  tmpout_= write(p->fd_, &version, sizeof(uint32_t));
  tmpout_= write(p->fd_, &magic_num, sizeof(uint32_t));
}

jp_db5::jp_db5(const char* fname, const char* mode)
{
  //assert (sizeof(entry_info_0_1)!=16); Relja: the assertion fails, problem?

  pimpl_ = new jp_db5_imp;

  pimpl_->fname_ = fname;
  pimpl_->mode_ = mode;

  if (strcmp(mode, "r")==0) { // Read-only
    pimpl_->fd_ = open(fname, O_RDONLY, (mode_t)0644);
    if (pimpl_->fd_==-1) throw std::runtime_error("IOError: can't open file");
    load_internal_data(pimpl_);
  }
  else if (strcmp(mode, "r+")==0) { // Read-write, don't create
    pimpl_->fd_ = open(fname, O_RDWR, (mode_t)0644);
    if (pimpl_->fd_==-1) throw std::runtime_error("IOError: can't open file");
    load_internal_data(pimpl_);
  }
  else if (strcmp(mode, "a")==0) { // Read-write, create if !exists
    if (fexists(fname)) {
      pimpl_->fd_ = open(fname, O_RDWR, (mode_t)0644);
      if (pimpl_->fd_==-1) throw std::runtime_error("IOError: can't open file");
      load_internal_data(pimpl_);
    }
    else {
      pimpl_->fd_ = open(fname, O_RDWR | O_CREAT, (mode_t)0644);
      if (pimpl_->fd_==-1) throw std::runtime_error("IOError: can't open file");
      pimpl_->bsize_ = 4096; // Hard coded for now.
      pimpl_->nblocks_ = 0;
    }
  }
  else if (strcmp(mode, "w")==0) { // Write, overwrite if exists
    pimpl_->fd_ = open(fname, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);
    if (pimpl_->fd_==-1) throw std::runtime_error("IOError: can't open file");
    pimpl_->bsize_ = 4096; // Hard coded for now.
    pimpl_->nblocks_ = 0;
  }

  // Tell the kernel that we expect access to be somewhat random.
  posix_fadvise(pimpl_->fd_, 0, 0, POSIX_FADV_RANDOM);
}

jp_db5::~jp_db5()
{
  if (pimpl_->fd_ != -1)
    close();
  delete pimpl_;
}

void
jp_db5::close()
{
  if (pimpl_->mode_ != "r") {
    save_internal_data(pimpl_);
  }
  ::close(pimpl_->fd_);
  pimpl_->fd_ = -1;
}

int
jp_db5::count(const char *key) const
{
  if ((pimpl_->key_to_info_).count(key)) return 1;
  else return 0;
}

void
jp_db5::get(const char *key, char **data, uint32_t *size) const
{
  jp_db5_imp::map_type::const_iterator it = (pimpl_->key_to_info_).find(key);
  if (it == (pimpl_->key_to_info_).end()) { // Key doesn't exist!!!
    throw std::runtime_error("KeyError: key not found");
  }
  else {
    uint32_t bnum = (it->second).bnum_; // Block number
    uint32_t extent = (it->second).extent_; // Extent in bytes

    int is_gziped = (it->second).is_gziped_;
    int is_lzoed = (it->second).is_lzoed_;

    char *new_data = (char*)malloc(extent);
    size_t new_data_size = extent;

    // We prefer doing just one thread-safe system call.
    ssize_t tmpout_= 0; // Relja: to remove warning that we're ignoring outputs
    ++tmpout_; // just to avoid "warning: variable ‘tmpout_’ set but not used [-Wunused-but-set-variable]"
    tmpout_= pread64(pimpl_->fd_, new_data, extent, (off64_t)pimpl_->bsize_ * bnum);

    if (is_gziped) {
      gzip_uncompress(&new_data, &new_data_size);
    }

    if (is_lzoed) {
      lzo_uncompress(&new_data, &new_data_size);
    }

    (*data) = new_data;
    (*size) = new_data_size;
  }
}

void
jp_db5::set(const char* key, const char *data, uint32_t size,
            int flags, int comp_level)
{
  char *buffer = (char*)malloc(size);
  memcpy(buffer, data, size);
  size_t buffer_size = size;

  if (flags & JP_DB5_GZIP) {
    gzip_compress(&buffer, &buffer_size, comp_level);
  }

  if (flags & JP_DB5_LZO) {
    lzo_compress(&buffer, &buffer_size);
  }

  uint32_t start_block = pimpl_->nblocks_;
  uint32_t end_block = start_block + (buffer_size + pimpl_->bsize_ - 1)/pimpl_->bsize_;

  //lseek64(pimpl_->fd_, (off64_t)pimpl_->bsize_ * start_block, SEEK_SET);
  //write(pimpl_->fd_, buffer, buffer_size);
  ssize_t tmpout_= 0; // Relja: to remove warning that we're ignoring outputs
  ++tmpout_; // just to avoid "warning: variable ‘tmpout_’ set but not used [-Wunused-but-set-variable]"
  tmpout_= pwrite64(pimpl_->fd_, buffer, buffer_size, (off64_t)pimpl_->bsize_ * start_block);

  entry_info_0_1 info;
  info.bnum_ = start_block;
  info.extent_ = buffer_size;
  info.attr_ = 0;
  info.is_gziped_ = (flags & JP_DB5_GZIP) ? 1 : 0;
  info.is_lzoed_ = (flags & JP_DB5_LZO) ? 1 : 0;

  pimpl_->key_to_info_[key] = info;

  pimpl_->nblocks_ = end_block;

  free(buffer);
}

size_t
jp_db5::size() const
{
  return pimpl_->key_to_info_.size();
}

class
jp_db5_iterator
{
public:
  const jp_db5 *p;

  jp_db5_imp::map_type::const_iterator cur_;
  jp_db5_imp::map_type::const_iterator end_;
};

void
jp_db5_iterator_advance(void *iter)
{
  jp_db5_iterator *p = (jp_db5_iterator*)iter;
  ++p->cur_;
}

int
jp_db5_iterator_finished(void *iter)
{
  jp_db5_iterator *p = (jp_db5_iterator*)iter;
  if (p->cur_ == p->end_)
    return 1;
  else
    return 0;
}

const char*
jp_db5_iterator_key(void *iter)
{
  jp_db5_iterator *p = (jp_db5_iterator*)iter;
  return p->cur_->first.c_str();
}

void
jp_db5_iterator_data(void *iter, char **data, uint32_t *size)
{
  jp_db5_iterator *p = (jp_db5_iterator*)iter;
  p->p->get(p->cur_->first.c_str(), data, size);
}

void*
jp_db5::get_iterator() const
{
  jp_db5_iterator* it = new jp_db5_iterator;

  it->p = this;
  it->cur_ = this->pimpl_->key_to_info_.begin();
  it->end_ = this->pimpl_->key_to_info_.end();

  return it;
}

void
jp_db5_del_iterator(void *iter){
    delete (jp_db5_iterator*)iter;
}
