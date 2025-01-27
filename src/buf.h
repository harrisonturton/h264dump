#ifndef BUF_H
#define BUF_H

#include <stdio.h>
#include <stdlib.h>

#include "attrs.h"

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
  void* nullable start;

  // One past the of the buffer.
  void* nullable end;

  // Current location in the buffer.
  void* nullable curr;

  // Last error encountered.
  error err;

  // Fetch new data.
  error (*nonnull refill)(struct buf* nonnull buf);

  // Free the data in the buffer.
  error (*nonnull free)(struct buf* nonnull buf);

  // Implementation-specific data.
  void* nonnull data;
};

/**
 * Create a new buffer from a file.
 */
error buf_file(struct buf* nonnull buf, const char* nonnull path);

/**
 * Refill the buffer with data.
 */
error buf_refill(struct buf* nonnull buf);

/**
 * Release all the resource associated with the buffer.
 */
error buf_free(struct buf* nonnull buf);

/**
 * Read an unsigned 8-bit integer from the buffer into val.
 *
 * When val is NULL, this pops 1 byte from the buffer.
 */
error buf_read_u8(struct buf* nonnull buf, uint32_t* nullable val);

/**
 * Read a big-endian 32-bit unsigned integer from the buffer.
 *
 * When val is NULL, this pops 4 bytes from the buffer.
 */
error buf_read_u32_be(struct buf* nonnull buf, uint32_t* nullable val);

/**
 * Read a big-endian 32-bit unsigned integer without incrementing the cursor.
 *
 * When val is NULL, this is a noop.
 */
error buf_peek_32_be(struct buf* nonnull buf, uint32_t* nullable val);

#endif
