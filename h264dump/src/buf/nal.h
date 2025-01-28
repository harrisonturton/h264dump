#ifndef BUF_NAL_H
#define BUF_NAL_H

#include "attrs.h"
#include "error.h"
#include "buf/buf.h"

/**
 * Create a NAL buffer from an existing source buffer.
 *
 * A NAL buffer will always be aligned on NAL unit boundaries.
 *
 * It guarantees that `buf->start` will be at the first byte of the NAL, and
 * that `buf->end` will either be partway through the NAL, or at the end.
 * 
 * After this function is called, the buffer in `src` should not be used,
 * because it will be mutated whenever `buf` is used.
 *
 * @param buf pointer to populate with the created buffer.
 * @param src source buffer to read data from.
 */
error buf_nal(struct buf* nonnull buf, struct buf* nonnull src);

#endif
