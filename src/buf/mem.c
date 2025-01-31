#include "buf/buf.h"
#include "common/compiler.h"
#include "common/result.h"

result_t buf_refill_zeroes(struct buf* nonnull buf) {
  static uint8_t zeroes[256] = {0};
  buf->start = zeroes;
  buf->curr = zeroes;
  buf->end = zeroes + sizeof(zeroes);
  return buf->res;
}

static result_t buf_mem_refill(struct buf* nonnull buf) {
  buf->refill = buf_refill_zeroes;
  buf->res = ERR_EOF;
  return buf->res;
}

static result_t buf_mem_free(struct buf* nonnull buf) {
  free(buf->ctx);
  free(buf);
  return OK;
}

result_t buf_mem(struct buf* nonnull buf, void* nonnull ptr, size_t len) {
  *buf = (struct buf){
      .start = ptr,
      .end = ptr + len,
      .curr = ptr,
      .ctx = ptr,
      .refill = buf_mem_refill,
      .free = buf_mem_free,
  };

  return OK;
}
