#include "common/compiler.h"
#include "common/result.h"
#include "reader/reader.h"

result_t refill_zeroes(struct reader* nonnull reader) {
  static uint8_t zeroes[256] = {0};
  reader->start = zeroes;
  reader->curr = zeroes;
  reader->end = zeroes + sizeof(zeroes);
  return reader->res;
}

static result_t mem_reader_refill(struct reader* nonnull reader) {
  reader->refill = refill_zeroes;
  reader->res = ERR_EOF;
  return reader->res;
}

static result_t mem_reader_free(struct reader* nonnull reader) {
  free(reader->ctx);
  free(reader);
  return OK;
}

result_t mem_reader(struct reader* nonnull reader,
                    void* nonnull ptr,
                    size_t len) {
  *reader = (struct reader){
      .start = ptr,
      .end = ptr + len,
      .curr = ptr,
      .ctx = ptr,
      .refill = mem_reader_refill,
      .free = mem_reader_free,
  };

  return OK;
}