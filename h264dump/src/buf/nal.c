#include <stdbool.h>

#include "buf.h"
#include "nal.h"

#define NAL_START_CODE_PREFIX_A 0x00000001
#define NAL_START_CODE_PREFIX_B 0x00000000
#define NAL_START_CODE_SEARCH_MAX 2048

struct nal_buf_ctx {
  // Whether the current fill is part of a NAL
  bool has_nal;

  // Inner buffer to read from
  struct buf* buf;
};

static error buf_nal_gobble_start_code(struct buf* nonnull nal_buf) {
  error err = ERR_NONE;
  struct nal_buf_ctx* ctx = buf_ctx(nal_buf);

  size_t iters = 0;
  while (iters < NAL_START_CODE_SEARCH_MAX) {
    uint32_t start_code;

    err = buf_peek_u32_be(ctx->buf, &start_code);
    if (err < 0) {
      return err;
    }

    if (start_code == NAL_START_CODE_PREFIX_A) {
      break;
    }

    err = buf_read_u8(ctx->buf, NULL);
    if (err < 0) {
      return err;
    }

    iters++;
  }

  if (iters == NAL_START_CODE_SEARCH_MAX) {
    return ERR_INVAL;
  }

  err = buf_read_u32_be(ctx->buf, NULL);
  if (err < 0) {
    return err;
  }

  nal_buf->start = ctx->buf->curr;
  return ERR_NONE;
}

static error buf_nal_gobble_end_code(struct buf* nonnull nal_buf) {
  error err = ERR_NONE;
  struct nal_buf_ctx* ctx = buf_ctx(nal_buf);

  size_t iters = 0;
  while (buf_rem(ctx->buf) >= 4 && iters < NAL_START_CODE_SEARCH_MAX) {
    uint32_t end_code;

    err = buf_read_u32_be(ctx->buf, &end_code);
    if (err < 0) {
      return err;
    }

    end_code &= 0x00ffffff;

    if (end_code == NAL_START_CODE_PREFIX_A) {
      break;
    }

    if (end_code == NAL_START_CODE_PREFIX_B) {
      break;
    }

    iters++;
  }

  if (iters == NAL_START_CODE_SEARCH_MAX) {
    return ERR_NOT_FOUND;
  }

  // TODO: Check if no end point found, but reached end of buffer. Suspect bug
  // in the buf_rem(nal_buf) interaction.
  nal_buf->end = ctx->buf->curr;
  return 0;
}

static error buf_nal_refill(struct buf* nonnull nal_buf) {
  error err = ERR_NONE;
  struct nal_buf_ctx* ctx = buf_ctx(nal_buf);

  err = buf_refill(ctx->buf);
  if (err < 0) {
    return err;
  }

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

  // TODO: Handle refills where the current refill straddles a NAL boundary. What then?
  return ERR_NONE;
}

static error buf_nal_free(struct buf* nonnull nal_buf) {
  free(nal_buf->ctx);
  return ERR_NONE;
}

error buf_nal(struct buf* nonnull buf, struct buf* nonnull src) {
  struct nal_buf_ctx* ctx = malloc(sizeof(struct nal_buf_ctx));
  if (ctx == NULL) {
    return ERR_NOMEM;
  }

  *ctx = (struct nal_buf_ctx){
      .has_nal = false,
      .buf = buf,
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
