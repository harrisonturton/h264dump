#include "h264/nal_reader.h"
#include <stdint.h>
#include <stdio.h>
#include "common/compiler.h"
#include "common/result.h"
#include "reader/reader.h"

result_t nal_reader_refill_annexb(struct reader* nonnull reader) {
  result_t res;
  struct nal_reader* ctx = reader->ctx;
  struct annexb_iter* iter = &ctx->iter.annexb;

  // When we've found the start and end code, we know we've consumed the reader
  // at least until the end of the current NALU. Reset to find another.
  if (iter->found_start_code && iter->found_end_code) {
    iter->found_start_code = false;
    iter->found_end_code = false;
  }

  // The source reader cursor will potentially be pushed until the end of the
  // current refill, but the end code might not exist in this refill. If that
  // happens, we need to reset the cursor, so we store it here.
  void* nal_start = NULL;
  void* nal_end = NULL;

  while (!iter->found_start_code) {
    uint32_t maybe_start_code;

    res = reader_peek_u32(ctx->src, &maybe_start_code, EDGE_MODE_REFILL);
    if (res < 0) {
      return res;
    }

    if (maybe_start_code == 0x1) {
      reader_advance(ctx->src, 4);
      iter->found_start_code = true;
      nal_start = ctx->src->curr;
      break;
    }

    reader_advance(ctx->src, 1);
  }

  while (!iter->found_end_code) {
    uint32_t maybe_end_code;

    enum edge_mode mode = nal_start == NULL ? EDGE_MODE_REFILL :
    EDGE_MODE_ERR; res = reader_peek_u32(ctx->src, &maybe_end_code, mode); if
    (res == OK_READ) {
      nal_start = ctx->src->start;
    }
    if (res == ERR_NO_MEM) {
      // Reached end of current refill, which isn't an error, but we need to
      // account for it when creating the pointers in the nal reader
      nal_end = ctx->src->end;
      break;
    }
    if (res < 0) {
      return res;
    }

    if (maybe_end_code == 0x1 || maybe_end_code == 0x0) {
      iter->found_end_code = true;
      nal_end = ctx->src->curr;
      break;
    }

    reader_advance(ctx->src, 1);
  }

  if (nal_start != NULL) {
    reader->start = nal_start;
    reader->curr = nal_start;
  }

  if (nal_end != NULL) {
    reader->end = nal_end;
  }

  return OK;
}

result_t nal_reader_refill_avcc(struct reader* nonnull reader) {
  return ERR_UNIMPLEMENTED;
}

result_t nal_reader_refill(struct reader* nonnull reader) {
  struct nal_reader* ctx = reader->ctx;
  switch (ctx->bitstream) {
    case ANNEXB:
      return nal_reader_refill_annexb(reader);
    case AVCC:
      return nal_reader_refill_avcc(reader);
    default:
      return ERR_INVALID_STATE;
  }
}

result_t nal_reader_free(struct reader* nonnull reader) {
  struct nal_reader* ctx = reader->ctx;
  reader_free(ctx->src);
  free(reader->ctx);
  free(reader);
  return ERR_UNIMPLEMENTED;
}

result_t nal_reader(struct reader* nonnull reader,
                    struct reader* nonnull src,
                    enum nal_bitstream bitstream) {
  struct nal_reader* nal_reader = calloc(1, sizeof(struct nal_reader));
  if (nal_reader == NULL) {
    return ERR_NO_MEM;
  }

  nal_reader->src = src;
  nal_reader->bitstream = bitstream;

  switch (bitstream) {
    case ANNEXB:
      nal_reader->iter.annexb = (struct annexb_iter){
          .found_start_code = false,
          .found_end_code = false,
      };
      break;
    case AVCC:
      nal_reader->iter.avcc = (struct avcc_iter){};
      break;
    default:
      free(nal_reader);
      return ERR_INVALID_STATE;
  }

  *reader = (struct reader){
      .start = NULL,
      .end = NULL,
      .curr = NULL,
      .res = reader->res,
      .ctx = nal_reader,
      .refill = nal_reader_refill,
      .free = nal_reader_free,
  };

  return OK;
}