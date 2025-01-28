#ifndef BUF_H
#define BUF_H

#include <stdio.h>
#include <stdlib.h>
#include "../error.h"
#include "../attrs.h"

/**
 * Producer-owned buffer abstraction.
 */
struct buf {
  // Start of the buffer.
  void* nullable start;

  // One past the of the buffer.
  void* nullable end;

  // Current location in the buffer.
  void* nullable curr;

  // Implementation-specific data.
  void* nullable ctx;

  // Last error encountered.
  error err;

  // Fetch new data.
  error (*nonnull refill)(struct buf* nonnull buf);

  // Free the data in the buffer.
  error (*nonnull free)(struct buf* nonnull buf);
};

/**
 * Refill the buffer with data.
 * 
 * @param buf buffer to refill.
 */
error buf_refill(struct buf* nonnull buf);

/**
 * Release all the resource associated with the buffer.
 * 
 * @param buf buffer to free.
 */
error buf_free(struct buf* nonnull buf);

/**
 * Get the number of remaining bytes in the current fill.
 *
 * @param buf buffer to read from.
 */
size_t buf_rem(struct buf* nonnull buf);

/**
 * Read an unsigned 8-bit integer from the buffer into val.
 *
 * When val is NULL, this pops 1 byte from the buffer.
 * 
 * @param buf buffer to read from.
 * @param val pointer to copy the data into.
 */
error buf_read_u8(struct buf* nonnull buf, uint32_t* nullable val);

/**
 * Read a big-endian 32-bit unsigned integer from the buffer.
 *
 * When val is NULL, this pops 4 bytes from the buffer.
 * 
 * @param buf buffer to read from.
 * @param val pointer to copy the data into.
 */
error buf_read_u32_be(struct buf* nonnull buf, uint32_t* nullable val);

/**
 * Read a big-endian 32-bit unsigned integer without incrementing the cursor.
 *
 * When val is NULL, this is a noop.
 * 
 * @param buf buffer to read from.
 * @param val pointer to copy the data into.
 */
error buf_peek_u32_be(struct buf* nonnull buf, uint32_t* nullable val);

#endif
