#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "attrs.h"
#include "buf.h"
#include "bytes.h"

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

  FILE* fp = buf->data;
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
  fclose(buf->data);
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
      .data = fp,
  };

  return ERR_NONE;
}

size_t buf_rem(struct buf* buf) {
  return buf->end - buf->start;
}

error buf_free(struct buf* buf) {
  return buf->free(buf);
}

error buf_refill(struct buf* buf) {
  return buf->refill(buf);
}

error buf_read_u8(struct buf* nonnull buf, uint32_t* nullable val) {
  if (buf->err != ERR_NONE) {
    return buf->err;
  }

  if (buf->curr == buf->end) {
    printf("Refilling\n");
    error err = buf_refill(buf);
    if (err < 0) {
      return err;
    }
  }

  uint8_t next = *(uint8_t*)buf->curr;
  buf->curr += 1;

  if (val != NULL) {
    *val = u32_be(next);
  }

  return ERR_NONE;
}

error buf_read_u32_be(struct buf* nonnull buf, uint32_t* nullable val) {
  if (buf->err != ERR_NONE) {
    return buf->err;
  }

  if (buf->curr == buf->end) {
    printf("Refilling\n");
    error err = buf_refill(buf);
    if (err < 0) {
      return err;
    }
  }

  size_t rem = buf_rem(buf);
  if (rem < 4) {
    uint8_t data[4] = {0};

    printf("leftovers detected\n");
    for (size_t i = 0; i < rem; i++) {
      data[i] = *(uint8_t*)buf->curr + i;
      buf->curr += 1;
    }

    assert((buf->curr == buf->end) && "Expected buf to be finished");

    error err = buf_refill(buf);
    if (err < 0) {
      return err;
    }

    size_t leftover = 4 - rem;
    for (size_t i = 0; i < leftover; i++) {
      data[rem + i - 1] = *(uint8_t*)buf->curr + 1;
      buf->curr += 1;
    }

    uint32_t next = *(uint32_t*)data;
    *val = u32_be(next);
    return ERR_NONE;
  }

  uint32_t next = *(uint32_t*)buf->curr;
  buf->curr += 4;

  if (val != NULL) {
    *val = u32_be(next);
  }

  return ERR_NONE;
}

error buf_peek_32_be(struct buf* nonnull buf, uint32_t* nullable val) {
  if (buf->err != ERR_NONE) {
    return buf->err;
  }

  if (buf->curr == buf->end) {
    printf("Refilling\n");
    error err = buf_refill(buf);
    if (err < 0) {
      return err;
    }
  }

  size_t rem = buf_rem(buf);
  if (rem < 4) {
    // Don't handle cross-refill boundaries for peeking
    return ERR_REFILL_FAILED;
  }

  uint32_t next = *(uint32_t*)buf->curr;
  if (val != NULL) {
    *val = u32_be(next);
  }

  return ERR_NONE;
}
