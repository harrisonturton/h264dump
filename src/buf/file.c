#include <stdio.h>
#include <stdlib.h>
#include "buf/buf.h"
#include "common/result.h"

struct buf_file {
  FILE* fp;
  size_t chunk_len;
};

static result_t buf_file_refill(struct buf* nonnull buf) {
  struct buf_file* ctx = buf->ctx;

  void* data = malloc(ctx->chunk_len);
  if (!data) {
    buf->res = ERR_NO_MEM;
    return buf->res;
  }

  FILE* fp = ctx->fp;
  if (feof(fp) != 0) {
    buf->res = ERR_EOF;
    free(data);
    return ERR_EOF;
  }

  size_t read = fread(data, 1, ctx->chunk_len, fp);
  if (read != ctx->chunk_len) {
    // Fail when read doesn't complete but not EOF
    if (feof(fp) == 0) {
      free(data);
      buf->res = ERR_NO_MEM;
      return buf->res;
    }

    // But if we did reach EOF then we expect that
    buf->start = data;
    buf->end = data + read;
    buf->curr = data;
    buf->res = OK;
    return buf->res;
  }

  buf->start = data;
  buf->end = data + ctx->chunk_len;
  buf->curr = data;
  buf->res = OK;

  return buf->res;
}

static result_t buf_file_free(struct buf* nonnull buf) {
  free(buf->start);
  fclose(buf->ctx);
  return OK;
}

result_t buf_file(struct buf* nonnull buf,
                  const char* nonnull path,
                  size_t chunk_len) {
  FILE* fp = fopen(path, "r");
  if (fp == NULL) {
    return ERR_NOT_FOUND;
  }

  struct buf_file* ctx = calloc(1, sizeof(struct buf_file));
  *ctx = (struct buf_file){
      .fp = fp,
      .chunk_len = chunk_len,
  };

  *buf = (struct buf){
      .start = NULL,
      .end = NULL,
      .curr = NULL,
      .res = OK,
      .refill = &buf_file_refill,
      .free = &buf_file_free,
      .ctx = ctx,
  };

  return OK;
}