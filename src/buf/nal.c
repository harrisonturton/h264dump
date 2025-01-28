#include <stdbool.h>

#include "buf.h"
#include "nal.h"

#define NAL_START_CODE_PREFIX_A 0x00000001
#define NAL_START_CODE_PREFIX_B 0x00000000
#define NAL_START_CODE_SEARCH_MAX 2048

struct nal_buf {
  bool has_nal;
  void* inner;
};

static error buf_nal_gobble_start_code(struct buf* nonnull nal_buf) {
  error err;

  size_t iters = 0;
  while (iters < NAL_START_CODE_SEARCH_MAX) {
    uint32_t start_code;

    err = buf_peek_u32_be(nal_buf, &start_code);
    if (err < 0) {
      return err;

      if (start_code == NAL_START_CODE_PREFIX_A) {
        break;
      }

      err = buf_read_u8(nal_buf, NULL);
      if (err < 0) {
        return err;
      }

      iters++;
    }

    err = buf_read_u32_be(nal_buf, NULL);
    if (err < 0) {
      return err;
    }
  }

  if (iters == NAL_START_CODE_SEARCH_MAX) {
    return ERR_INVAL;
  }

  return ERR_NONE;
}

static error buf_nal_gobble_end_code(struct buf* nonnull nal_buf) {
  error err = ERR_NONE;

  size_t iters = 0;
  while (buf_rem(nal_buf) >= 4 && iters < NAL_START_CODE_SEARCH_MAX) {
    uint32_t next;
    err = buf_read_u32_be(nal_buf, &next);
    if (err < 0) {
      return err;
    }

    next &= 0x00ffffff;

    if (next == NAL_START_CODE_PREFIX_A) {
      break;
    }

    if (next == NAL_START_CODE_PREFIX_B) {
      break;
    }

    iters++;
  }

  if (iters == NAL_START_CODE_SEARCH_MAX) {
    return ERR_NOT_FOUND;
  }

  return 0;
}

static error buf_nal_refill(struct buf* nonnull nal_buf) {
  error err = ERR_NONE;

  struct nal_buf* ctx = (struct nal_buf*)nal_buf->ctx;

  // If nal_buf->nal is false (indicating we're not currently in a NAL) then
  // find start code Otherwise skip, and look for end code
  // TODO: Accelerate with SIMD
  err = buf_nal_gobble_start_code(nal_buf);
  if (err < 0) {
    return err;
  }

  // Look for end code in current refill
  // If found, set buf end pointer to be that byte (must cache buf pointer to
  // free later) If not found, set buf end pointer to be the current last byte
  err = buf_nal_gobble_end_code(nal_buf);
  if (err < 0) {
    return err;
  }

  return 0;
}

static error buf_nal_free(struct buf* nonnull nal_buf) {
  free(nal_buf->ctx);
  return ERR_NONE;
}

error buf_nal(struct buf* nonnull buf, struct buf* nonnull src) {
  struct nal_buf* ctx = malloc(sizeof(struct nal_buf));
  if (ctx == NULL) {
    return ERR_NOMEM;
  }

  *ctx = (struct nal_buf){
      .has_nal = false,
      .inner = buf->start,
  };

  *buf = (struct buf){
      .start = NULL,
      .end = NULL,
      .curr = NULL,
      .ctx = ctx,
      .err = ERR_NONE,
      .refill = &buf_nal_refill,
      .free = &buf_nal_free,
  };

  return ERR_NONE;
}
