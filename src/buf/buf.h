#ifndef buf_h
#define buf_h

#include <stdlib.h>
#include "common/compiler.h"
#include "common/result.h"

struct buf {
  void* nullable start;
  void* nullable end;
  void* nullable curr;
  void* nonnull ctx;
  result_t res;
  result_t (*nonnull refill)(struct buf* nonnull buf);
  result_t (*nonnull free)(struct buf* nonnull buf);
};

enum edge_mode {
  EDGE_MODE_REFILL = 0,
  EDGE_MODE_ERR = 1,
};

/**
 * Create a buffer from a region of memory. Will EOF upon refill.
 */
result_t buf_mem(struct buf* nonnull buf, void* nonnull ptr, size_t len);

/**
 * Create a buffer from a file. Will defer read length to libc when chunk_len is -1.
 */
result_t buf_file(struct buf* nonnull buf, const char* nonnull path, size_t chunk_len);

/**
 * Refill the buffer with memory from the source. Replaces the current fill.
 */
result_t buf_refill(struct buf* nonnull buf);

/**
 * Current read offset in the buffer.
 */
size_t buf_curr(const struct buf* nonnull buf);

/**
 * Number of remaining bytes after the read offset.
 */
size_t buf_avail(const struct buf* nonnull buf);

/**
 * Total length of the current buffer. May change upon refill.
 */
size_t buf_len(const struct buf* nonnull buf);

/**
 * Release all memory owned by the buffer.
 */
result_t buf_free(struct buf* nonnull buf);

/**
 * Extract an 8-bit integer and increment the read offset.
 */
result_t buf_read_u8(struct buf* nonnull buf,
                      uint8_t* nonnull val,
                      enum edge_mode mode);

/**
 * Extract a 24-bit big endian integer and increment the read offset.
 */
result_t buf_read_u24(struct buf* nonnull buf,
                      uint32_t* nonnull val,
                      enum edge_mode mode);

/**
 * Extract a 23-bit big endian integer and increment the read offset.
 */
result_t buf_read_u32(struct buf* nonnull buf,
                      uint32_t* nonnull val,
                      enum edge_mode mode);

/**
 * Extract an 8-bit integer without incrementing the read offset.
 */
result_t buf_peek_u8(struct buf* nonnull buf,
                      uint8_t* nonnull val,
                      enum edge_mode mode);

/**
 * Extract a 24-bit integer without incrementing the read offset.
 */
result_t buf_peek_u24(struct buf* nonnull buf,
                      uint32_t* nonnull val,
                      enum edge_mode mode);

/**
 * Extract a 32-bit integer without incrementing the read offset.
 */
result_t buf_peek_u32(struct buf* nonnull buf,
                      uint32_t* nonnull val,
                      enum edge_mode mode);

#endif