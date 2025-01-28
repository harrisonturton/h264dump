#include "buf.h"
#include "file.h"

#define CHUNK_SZ 256

static void buf_set_err(struct buf* nonnull buf, error err) {
  buf->start = NULL;
  buf->curr = NULL;
  buf->end = NULL;
  buf->err = err;
}

static error buf_file_refill(struct buf* nonnull buf) {
  void* data = malloc(CHUNK_SZ);
  if (!data) {
    buf_set_err(buf, ERR_NOMEM);
    return ERR_NOMEM;
  }

  FILE* fp = buf->ctx;
  if (feof(fp) != 0) {
    buf_set_err(buf, ERR_EOF);
    free(data);
    return ERR_EOF;
  }

  size_t read = fread(data, 1, CHUNK_SZ, fp);
  if (read != CHUNK_SZ) {
    // Fail when read doesn't complete but not EOF
    if (feof(fp) == 0) {
      free(data);
      buf_set_err(buf, ERR_REFILL_FAILED);
      return ERR_REFILL_FAILED;
    }

    // But if we did reach EOF then we expect that
    buf->start = data;
    buf->end = data + read;
    buf->curr = data;
    buf->err = ERR_NONE;
    return ERR_NONE;
  }

  buf->start = data;
  buf->end = data + CHUNK_SZ;
  buf->curr = data;
  buf->err = ERR_NONE;

  return ERR_NONE;
}

static error buf_file_free(struct buf* nonnull buf) {
  free(buf->start);
  fclose(buf->ctx);
  return ERR_NONE;
}

error buf_file(struct buf* nonnull buf, const char* nonnull path) {
  FILE* fp = fopen(path, "r");
  if (fp == NULL) {
    return ERR_NOT_FOUND;
  }

  *buf = (struct buf){
      .start = NULL,
      .end = NULL,
      .curr = NULL,
      .err = ERR_NONE,
      .refill = &buf_file_refill,
      .free = &buf_file_free,
      .ctx = fp,
  };

  return ERR_NONE;
}
