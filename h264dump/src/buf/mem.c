
#include "mem.h"
#include "attrs.h"
#include "buf.h"
#include "error.h"

static error buf_refill_zeroes(struct buf* nonnull buf) {
  static uint8_t zeroes[256] = {0};
  buf->start = &zeroes;
  buf->curr = &zeroes;
  buf->end = &zeroes + sizeof(zeroes);
  return buf->err;
}

static error buf_mem_refill(struct buf* nonnull buf) {
  buf->err = ERR_EOF;
  return buf_refill_zeroes(buf);
}

static error buf_mem_free(struct buf* nonnull buf) {
  free(buf->ctx);
  return ERR_NONE;
}

error buf_mem(struct buf* nonnull buf, void* nonnull ptr, size_t len) {
  *buf = (struct buf){
      .start = ptr,
      .end = ptr + len,
      .curr = ptr,
      .ctx = ptr,
      .err = ERR_NONE,
      .refill = buf_mem_refill,
      .free = buf_mem_free,
  };

  return ERR_NONE;
}
