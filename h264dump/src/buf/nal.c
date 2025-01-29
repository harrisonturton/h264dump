#include <stdbool.h>
#include <time.h>
#include <stdarg.h>

#include "buf.h"
#include "nal.h"

#define NAL_START_CODE_PREFIX_A 0x00000001
#define NAL_START_CODE_PREFIX_B 0x00000000
#define NAL_START_CODE_SEARCH_MAX 620

static bool run = false;

static void delay_input() {
  if (!run) {
    char ch = getchar();
    if (ch == 'r') {
      run = true;
    }
  }
}

static void delay_long() {
  volatile long i;
  for (i = 0; i < 1000000000; i++) {
    asm volatile("");
  }
}

static void delay_short() {
  volatile long i;
  for (i = 0; i < 10000000; i++) {
    asm volatile("");
  }
}

static void debug_log(struct buf* nonnull src, const char* format, ...) {
    size_t offset = buf_curr(src);

    // Start processing the variable arguments
    va_list args;
    va_start(args, format);

    // Print the debug log with the offset followed by the formatted message
    printf("[0x%08zx] ", offset);
    vprintf(format, args);

    // End the variable arguments list
    va_end(args);
}

static error gobble_until_start_code(struct buf* nonnull src) {
  error err = ERR_NONE;

  size_t iters = 0;
  while (iters < NAL_START_CODE_SEARCH_MAX) {
    uint32_t start_code;

    err = buf_peek_u32_be(src, &start_code);
    if (err < 0) {
      return err;
    }

    debug_log(src, "start peek: 0x%08x", start_code);
    delay_input();

    if (start_code == NAL_START_CODE_PREFIX_A) {
      break;
    }

    uint32_t masked_start_code = (start_code >> 8) & 0x00ffffff;
    if (masked_start_code == NAL_START_CODE_PREFIX_A) {
      src->curr--;
      break;
    }

    err = buf_read_u8(src, NULL);
    if (err < 0) {
      return err;
    }

    iters++;
  }

  run = false;

  if (iters == NAL_START_CODE_SEARCH_MAX) {
    return ERR_INVAL;
  }

  err = buf_read_u32_be(src, NULL);
  if (err < 0) {
    return err;
  }

  return 1;
}

static error gobble_until_end_code(struct buf* nonnull src) {
  error err = ERR_NONE;

  size_t iters = 0;
  while (iters < NAL_START_CODE_SEARCH_MAX) {
    uint32_t end_code;

    delay_input();

    err = buf_read_u32_be(src, &end_code);
    if (err < 0) {
      return err;
    }

    src->curr -= 3;
    end_code = (end_code >> 8) & 0x00ffffff;
    debug_log(src, "end_code: 0x%x", end_code);

    if (end_code == NAL_START_CODE_PREFIX_A ||
        end_code == NAL_START_CODE_PREFIX_B) {
      run = false;
      src->curr -= 1;
      return 1;
    }

    iters++;
  }

  run = false;

  if (iters == NAL_START_CODE_SEARCH_MAX) {
    // printf("reached max search\n");
    return ERR_NOT_FOUND;
  }

  return 0;
}

static error buf_nal_refill(struct buf* nonnull nal_buf) {
  error err = ERR_NONE;
  struct nal_buf_ctx* ctx = buf_ctx(nal_buf);

  printf("Refilling NAL buffer\n");

  // If we've already encountered a start and end code, then this refill must
  // search for the beginning of a new NAL in the source buffer.
  if (ctx->found_start_code && ctx->found_end_code) {
    ctx->src->curr = nal_buf->end;
    ctx->found_start_code = false;
    ctx->found_end_code = false;
  }

  // Only refill from the source buffer when we've exhausted it, because we
  // might find multiple NAL units within one source buffer refill.
  if (buf_rem(ctx->src) == 0) {
    err = buf_refill(ctx->src);
    if (err < 0) {
      printf("NAL source buffer refill failed\n");
      return err;
    }
  }

  // If we're still searching for a new start code, consume bytes from the
  // source buffer until we find it, refilling when needing.
  if (!ctx->found_start_code) {
    err = gobble_until_start_code(ctx->src);
    if (err < 0) {
      printf("Gobble till start failed\n");
      return err;
    }

    ctx->found_start_code = true;
  }

  size_t offset = ctx->src->curr - ctx->src->start;
  debug_log(ctx->src, "Found start offset: %lu\n", offset);

  // At this point, the cursor in the source buffer is either pointing to the
  // first byte, when we didn't search for the start code, or the byte after the
  // start code. Either way, we're ready to construct the view.
  nal_buf->start = ctx->src->curr;
  nal_buf->curr = ctx->src->curr;

  // Look for the end code in the current refill. If we don't find it, we know
  // the NAL spans a refill boundary.
  err = gobble_until_end_code(ctx->src);
  if (err < 0) {
    printf("Gobble till end failed\n");
    return err;
  }

  if (err == 1) {
    ctx->found_end_code = true;
    nal_buf->end = ctx->src->curr;

    printf("\nfound end code\n");
    printf("cursor: %lu/%lu\n", buf_curr(ctx->src), buf_len(ctx->src));

    return ERR_NONE;
  }

  nal_buf->end = ctx->src->end;
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
      .offset = 0,
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
