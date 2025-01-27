#ifndef BUF_H
#define BUF_H

#include <stdio.h>
#include <stdlib.h>

typedef enum _error {
  ERR_NONE = 0,
  ERR_NOT_FOUND = -1,
  ERR_NOMEM = -2,
  ERR_EOF = -3,
  ERR_REFILL_FAILED = -4,
} error;

/**
 * Refillable buffer abstraction.
 */
struct buf {
  // Start of the buffer.
  void* start;

  // One past the of the buffer.
  void* end;

  // Current location in the buffer.
  void* curr;

  // Last error encountered.
  error err;

  // Fetch new data.
  error (*refill)(struct buf* buf);

  // Free the data in the buffer.
  error (*free)(struct buf* buf);

  // Implementation-specific data.
  void* data;
};

/**
 * Create a new buffer from a file.
 */
error buf_file(struct buf* buf, const char* path);

/**
 * Refill the buffer with data.
 */
error buf_refill(struct buf* buf);

/**
 * Release all the resource associated with the buffer. 
 */
error buf_free(struct buf* buf);

/**
 * Read a big-endian 32-bit unsigned integer from the buffer.
 */
error buf_read_u32_be(struct buf* buf, uint32_t *val);

#endif