/**
 * Implements a simple database.
 */

#ifndef __JP_DB5_HPP
#define __JP_DB5_HPP

#include <stdint.h>
#include <stdexcept>

#define JP_DB5_GZIP 0x1
#define JP_DB5_LZO 0x2

class jp_db5_imp;

class
jp_db5
{
private:
  jp_db5_imp *pimpl_;

public:
  /**
   * Open \c fname for writing. \c mode can be one of 'r', 'r+', 'a', 'w'.
   */
  jp_db5(const char *fname, const char *mode = "r");

  ~jp_db5();

  /**
   * Closes the file saving all necessary data.
   */
  void
  close();

  /**
   * Returns 1 iff the key is present.
   */
  int
  count(const char *key) const;

  /**
   * Get the data associated with \c key. The data is stored in a new
   * pointer assigned to \c data. Caller takes responsibility for
   * calling free.
   */
  void
  get(const char *key, char **data, uint32_t *size) const;

  /**
   * Store the data at location \c data with size \c size assigning it
   * to key \c key. If \c key already exists, it is overwritten.
   *
   * \c flags can be one or more of JP_DB5_SHUFFLE, JP_DB5_GZIP.
   */
  void
  set(const char *key, const char *data, uint32_t size,
      int flags, int comp_level);

  /**
   * The number of (key, value) pairs stored in this database.
   */
  size_t
  size() const;

  /**
   * Returns an iterator for this database.
   */
  void*
  get_iterator() const;
};

/**
 * Advances the iterator.
 */
void
jp_db5_iterator_advance(void *iter);

int
jp_db5_iterator_finished(void *iter);

/**
 * Returns the key of the current iterator position.
 */
const char*
jp_db5_iterator_key(void *iter);

/**
 * Reads the data for the current iterator position. Caller takes
 * responsibility for calling free.
 */
void
jp_db5_iterator_data(void *iter, char **data, uint32_t *size);

void
jp_db5_del_iterator(void *iter);

#endif
