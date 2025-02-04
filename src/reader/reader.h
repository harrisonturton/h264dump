#ifndef reader_h
#define reader_h

#include <stdlib.h>
#include "common/compiler.h"
#include "common/result.h"

struct reader {
  void* nullable start;
  void* nullable end;
  void* nullable curr;
  void* nonnull ctx;
  result_t res;
  result_t (*nonnull refill)(struct reader* nonnull reader);
  result_t (*nonnull free)(struct reader* nonnull reader);
};

enum edge_mode {
  EDGE_MODE_REFILL = 0,
  EDGE_MODE_ERR = 1,
};

/**
 * Create a reader from a region of memory. Will EOF upon refill.
 */
result_t mem_reader(struct reader* nonnull reader,
                    void* nonnull ptr,
                    size_t len);

/**
 * Create a reader from a file that reads in chunk_len byte units.
 */
result_t file_reader(struct reader* nonnull reader,
                     const char* nonnull path,
                     size_t chunk_len);

/**
 * Refill the readerfer with memory from the source. Replaces the current fill.
 */
result_t reader_refill(struct reader* nonnull reader);

/**
 * Current read offset in the readerfer.
 */
size_t reader_curr(const struct reader* nonnull reader);

/**
 * Number of remaining bytes after the read offset.
 */
size_t reader_avail(const struct reader* nonnull reader);

/**
 * Advance the reader N bytes.
 */
result_t reader_advance(struct reader* nonnull reader, size_t n);

/**
 * Total length of the current readerfer. May change upon refill.
 */
size_t reader_len(const struct reader* nonnull reader);

/**
 * Release all memory owned by the readerfer.
 */
result_t reader_free(struct reader* nonnull reader);

/**
 * Extract an 8-bit integer and increment the read offset.
 */
result_t reader_read_u8(struct reader* nonnull reader,
                        uint8_t* nonnull val,
                        enum edge_mode mode);

/**
 * Extract a 24-bit big endian integer and increment the read offset.
 */
result_t reader_read_u24(struct reader* nonnull reader,
                         uint32_t* nonnull val,
                         enum edge_mode mode);

/**
 * Extract a 23-bit big endian integer and increment the read offset.
 */
result_t reader_read_u32(struct reader* nonnull reader,
                         uint32_t* nonnull val,
                         enum edge_mode mode);

/**
 * Extract an 8-bit integer without incrementing the read offset.
 */
result_t reader_peek_u8(struct reader* nonnull reader,
                        uint8_t* nonnull val,
                        enum edge_mode mode);

/**
 * Extract a 24-bit integer without incrementing the read offset.
 */
result_t reader_peek_u24(struct reader* nonnull reader,
                         uint32_t* nonnull val,
                         enum edge_mode mode);

/**
 * Extract a 32-bit integer without incrementing the read offset.
 */
result_t reader_peek_u32(struct reader* nonnull reader,
                         uint32_t* nonnull val,
                         enum edge_mode mode);

#endif