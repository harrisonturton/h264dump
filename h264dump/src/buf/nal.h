#ifndef buf_nal_h
#define buf_nal_h

#include <stdbool.h>
#include "attrs.h"
#include "error.h"
#include "buf/buf.h"
#include "attrs.h"

struct nal_buf_ctx {
  // Inner buffer to read from
  struct buf* nonnull src;

  // Whether the current fill is part of a NAL
  bool found_start_code;

  // Whether we've found the end of the current NAL. If this is false and
  // found_start_code is true, that means the NAL spans a refill boundary. 
  bool found_end_code;

  bool start_aligned;

  size_t offset;
};

/**
 * Create a NAL buffer from an existing source buffer.
 *
 * A NAL buffer will always be aligned on NAL unit boundaries. The buffer must
 * be refilled before it can be used.
 *
 * It guarantees that `buf->start` will be at the first byte of the NAL, and
 * that `buf->end` will either be partway through the NAL, or at the end.
 *
 * After this function is called, the NAL buffer takes ownership of the `src`
 * buffer. It must not continue to be used, because the NAL buffer will read and
 * write to it.
 * 
 * @param buf pointer to populate with the created buffer.
 * @param src source buffer to read data from.
 */
error buf_nal(struct buf* nonnull buf, struct buf* nonnull src);

#endif
