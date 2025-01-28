#ifndef BUF_H
#define BUF_H

#include <stdio.h>
#include <stdlib.h>
#include "attrs.h"
#include "error.h"

/**
 * Producer-owned buffer abstraction.
 *
 * A buffer is typically constructed through source-specific functions like
 * `buf_file` or `buf_nal`, though this header also provides generic utilities,
 * like functions to read primitive values.
 *
 * While the function pointers can be called directly, it is recommended to use
 * the accessor functions like `buf_refil` instead, to allow the evolution of
 * the buffer struct.
 */
struct buf {
  // Start of the buffer.
  void* nullable start;

  // One past the of the buffer.
  void* nullable end;

  // Current location in the buffer.
  void* nullable curr;

  // Implementation-specific data.
  void* nonnull ctx;

  // Last error encountered.
  error err;

  // Fetch new data.
  error (*nonnull refill)(struct buf* nonnull buf);

  // Free the data in the buffer.
  error (*nonnull free)(struct buf* nonnull buf);
};

/**
 * Extract the buffer context.
 *
 * @param buf buffer to extract the context from.
 */
void* nonnull buf_ctx(struct buf* nonnull buf);

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
