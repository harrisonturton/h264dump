#include <stdbool.h>

#include "buf.h"
#include "nal.h"

#define NAL_START_CODE_PREFIX_A 0x00000001
#define NAL_START_CODE_PREFIX_B 0x00000000
#define NAL_START_CODE_SEARCH_MAX 620

static error buf_nal_refill(struct buf* nonnull nal_buf) {
  error err = ERR_NONE;

  struct nal_buf_ctx* ctx = buf_ctx(nal_buf);
  struct buf* src = ctx->src;

  // Initial startup process to align to 4-byte [00,00,00,01] sequence.

  if (!ctx->start_aligned) {
    for (;;) {
      uint32_t start_code;

      err = buf_peek_u32_be(src, &start_code);
      if (err < 0) {
        return err;
      }

      if (start_code == 0x1) {
        break;
      }

      err = buf_read_u8(src, NULL);
      if (err < 0) {
        return err;
      }
    }

    ctx->start_aligned = true;
  }

  // Now, process that will be repeated for each NALU

  uint32_t next_four_bytes;

  err = buf_peek_u32_be(src, &next_four_bytes);
  if (err < 0) {
    return err;
  }

  // When the next four bytes in the bitstream form the four-byte sequence
  // 0x00000001, the next byte in the byte stream is extracted and discarded and
  // the current position in the byte stream is set equal to the position of the
  // byte following this discarded byte.
  if (next_four_bytes == 0x1) {
    err = buf_read_u8(src, NULL);
    if (err < 0) {
      return err;
    }
  }

  // The next three-byte sequence in the byte stream (which is a
  // start_code_prefix_one_3bytes) is extracted and discarded and the current
  // position in the byte stream is set equal to the position of the byte
  // following this three-byte sequence.
  src->curr += 3;
  void* nal_start = src->curr;

  // NumBytesInNALunit is set equal to the number of bytes starting with the
  // byte at the current position in the byte stream up to and including the
  // last byte that precedes the location of any of the following:
  //
  // – A subsequent byte-aligned three-byte sequence equal to 0x000000
  // – A subsequent byte-aligned three-byte sequence equal to 0x000001
  // – The end of the byte stream, as determined by unspecified means.
  uint32_t next_three_bytes;

pop_nal_body_byte:
  printf("Peeking body bytes\n");

  err = buf_peek_u32_be(src, &next_three_bytes);
  if (err < 0) {
    if (err == ERR_EOF) {
      goto found_end;
    }
    return err;
  }

  printf("peek success\n");

  next_three_bytes = (next_three_bytes >> 8) & 0xffffff;

  // If not found the end code, pop byte and continue looking
  if (next_three_bytes != 0x0 && next_three_bytes != 0x1) {
    err = buf_read_u8(src, NULL);
    if (err < 0) {
      return err;
    }

    goto pop_nal_body_byte;
  }

  // NumBytesInNALunit bytes are removed from the bitstream and the current
  // position in the byte stream is advanced by NumBytesInNALunit bytes. This
  // sequence of bytes is nal_unit( NumBytesInNALunit ) and is decoded using the
  // NAL unit decoding process.
  void* nal_end = src->curr;
  size_t nal_len = nal_end - nal_start;
  printf("found nal with length: %lu\n", nal_len);

found_end:
  nal_buf->start = nal_start;
  nal_buf->end = nal_end;
  nal_buf->curr = nal_start;

  // When the current position in the byte stream is not at the end of the byte
  // stream (as determined by unspecified means) and the next bytes in the byte
  // stream do not start with a three-byte sequence equal to 0x000001 and the
  // next bytes in the byte stream do not start with a four byte sequence equal
  // to 0x00000001, the decoder extracts and discards each trailing_zero_8bits
  // syntax element, moving the current position in the byte stream forward one
  // byte at a time, until the current position in the byte stream is such that
  // the next bytes in the byte stream form the fourbyte sequence 0x00000001 or
  // the end of the byte stream has been encountered (as determined by
  // unspecified means).

read_end:
  err = buf_peek_u32_be(src, &next_four_bytes);
  if (err < 0) {
    return err;
  }

  next_three_bytes = (next_four_bytes >> 8) & 0xffffff;
  if (next_three_bytes != 0x1 && next_four_bytes != 0x1) {
    err = buf_read_u8(src, NULL);
    if (err < 0) {
      return err;
    }

    goto read_end;
  }

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
      .src = src,
      .found_start_code = false,
      .found_end_code = false,
      .start_aligned = false,
      .offset = 0,
  };

  *buf = (struct buf){
      .start = src->start,
      .end = src->start,
      .curr = src->start,
      .ctx = ctx,
      .err = ERR_NONE,
      .refill = &buf_nal_refill,
      .free = &buf_nal_free,
  };

  return ERR_NONE;
}
