#include <stdio.h>
#include <stdlib.h>
#include "reader/reader.h"
#include "common/result.h"
#include "common/compiler.h"

struct file_reader {
  FILE* fp;
  size_t chunk_len;
};

static result_t file_reader_refill(struct reader* nonnull reader) {
  struct file_reader* ctx = reader->ctx;

  free(reader->start);

  void* data = malloc(ctx->chunk_len);
  if (!data) {
    reader->res = ERR_NO_MEM;
    return reader->res;
  }

  FILE* fp = ctx->fp;
  if (feof(fp) != 0) {
    reader->res = ERR_EOF;
    free(data);
    return ERR_EOF;
  }

  size_t read = fread(data, 1, ctx->chunk_len, fp);
  if (read != ctx->chunk_len) {
    // Fail when read doesn't complete but not EOF
    if (feof(fp) == 0) {
      free(data);
      reader->res = ERR_NO_MEM;
      return reader->res;
    }

    // But if we did reach EOF then we expect that
    reader->start = data;
    reader->end = data + read;
    reader->curr = data;
    reader->res = OK;
    return reader->res;
  }

  reader->start = data;
  reader->end = data + ctx->chunk_len;
  reader->curr = data;
  reader->res = OK;

  return reader->res;
}

static result_t file_reader_free(struct reader* nonnull reader) {
  free(reader->start);
  fclose(reader->ctx);
  return OK;
}

result_t file_reader(struct reader* nonnull reader,
                  const char* nonnull path,
                  size_t chunk_len) {
  FILE* fp = fopen(path, "r");
  if (fp == NULL) {
    return ERR_NOT_FOUND;
  }

  struct file_reader* ctx = calloc(1, sizeof(struct file_reader));
  *ctx = (struct file_reader){
      .fp = fp,
      .chunk_len = chunk_len,
  };

  *reader = (struct reader){
      .start = NULL,
      .end = NULL,
      .curr = NULL,
      .res = OK,
      .refill = &file_reader_refill,
      .free = &file_reader_free,
      .ctx = ctx,
  };

  return OK;
}