#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../attrs.h"
#include "../bytes.h"
#include "buf.h"

size_t buf_rem(struct buf* buf) {
  return buf->end - buf->start;
}

error buf_free(struct buf* buf) {
  return buf->free(buf);
}

error buf_refill(struct buf* buf) {
  return buf->refill(buf);
}

error buf_read_u8(struct buf* nonnull buf, uint32_t* nullable val) {
  if (buf->err != ERR_NONE) {
    return buf->err;
  }

  if (buf->curr == buf->end) {
    error err = buf_refill(buf);
    if (err < 0) {
      return err;
    }
  }

  uint8_t next = *(uint8_t*)buf->curr;
  buf->curr += 1;

  if (val != NULL) {
    *val = u32_be(next);
  }

  return ERR_NONE;
}

error buf_read_u32_be(struct buf* nonnull buf, uint32_t* nullable val) {
  if (buf->err != ERR_NONE) {
    return buf->err;
  }

  if (buf->curr == buf->end) {
    error err = buf_refill(buf);
    if (err < 0) {
      return err;
    }
  }

  size_t rem = buf_rem(buf);
  if (rem < 4) {
    uint8_t data[4] = {0};

    for (size_t i = 0; i < rem; i++) {
      data[i] = *(uint8_t*)buf->curr + i;
      buf->curr += 1;
    }

    assert((buf->curr == buf->end) && "Expected buf to be finished");

    error err = buf_refill(buf);
    if (err < 0) {
      return err;
    }

    size_t leftover = 4 - rem;
    for (size_t i = 0; i < leftover; i++) {
      data[rem + i - 1] = *(uint8_t*)buf->curr + 1;
      buf->curr += 1;
    }

    uint32_t next = *(uint32_t*)data;
    *val = u32_be(next);
    return ERR_NONE;
  }

  uint32_t next = *(uint32_t*)buf->curr;
  buf->curr += 4;

  if (val != NULL) {
    *val = u32_be(next);
  }

  return ERR_NONE;
}

error buf_peek_u32_be(struct buf* nonnull buf, uint32_t* nullable val) {
  if (buf->err != ERR_NONE) {
    return buf->err;
  }

  if (buf->curr == buf->end) {
    error err = buf_refill(buf);
    if (err < 0) {
      return err;
    }
  }

  size_t rem = buf_rem(buf);
  if (rem < 4) {
    // TODO: handle cross-refill boundaries when peeking
    return ERR_REFILL_FAILED;
  }

  uint32_t next = *(uint32_t*)buf->curr;
  if (val != NULL) {
    *val = u32_be(next);
  }

  return ERR_NONE;
}
