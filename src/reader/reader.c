#include "reader/reader.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "common/compiler.h"
#include "common/endian.h"
#include "common/result.h"

result_t reader_refill(struct reader* nonnull reader) {
  return reader->refill(reader);
}

size_t reader_curr(const struct reader* nonnull reader) {
  return reader->curr - reader->start;
}

size_t reader_avail(const struct reader* nonnull reader) {
  return reader->end - reader->curr;
}

result_t reader_advance(struct reader* nonnull reader, size_t n) {
  if (reader_avail(reader) < n) {
    return ERR_NO_MEM;
  }

  reader->curr += n;
  return OK;
}

size_t reader_len(const struct reader* nonnull reader) {
  return reader->end - reader->start;
}

result_t reader_free(struct reader* nonnull reader) {
  return reader->free(reader);
}

static result_t __read_refill(struct reader* nonnull reader,
                              void* nonnull ptr,
                              size_t len,
                              bool peek) {
  size_t avail = reader_avail(reader);
  size_t missing = len - avail;

  // Copy as many bytes as we can into ptr
  memcpy(ptr, reader->curr, avail);

  // Refill to get the remaining ones
  result_t res = reader_refill(reader);
  if (res < 0) {
    reader->res = res;
    return res;
  }

  // If we still don't have enough data after a refill, give up
  if (reader_avail(reader) < missing) {
    printf(
        "reader did not have enough data to peek a u32 even after refilling\n");
    return ERR_NO_MEM;
  }

  // Copy the remaining bytes into ptr
  memcpy(ptr + avail, reader->start, missing);

  if (!peek) {
    reader->curr += missing;
  }

  printf("__read_refill return OK\n");
  return OK;
}

static result_t __read_u8_refill(struct reader* nonnull reader,
                                 uint8_t* nonnull val,
                                 bool peek) {
  uint8_t data = 0;

  result_t res = __read_refill(reader, &data, 1, peek);
  if (res < 0) {
    reader->res = res;
    return res;
  }

  *val = data;
  return OK;
}

result_t reader_read_u8(struct reader* nonnull reader,
                        uint8_t* nonnull val,
                        enum edge_mode mode) {
  if (reader->res < 0) {
    return reader->res;
  }

  if (unlikely(reader_avail(reader) < sizeof(uint32_t))) {
    switch (mode) {
      case EDGE_MODE_ERR:
        reader->res = ERR_NO_MEM;
        return ERR_NO_MEM;
      case EDGE_MODE_REFILL:
        return __read_u8_refill(reader, val, false);
      default:
        reader->res = ERR_INVALID_INPUT;
        return ERR_INVALID_INPUT;
    }
  }

  uint8_t next = *(uint8_t*)reader->curr;
  reader->curr += 1;

  if (val != NULL) {
    *val = next;
  }

  return OK;
}

result_t reader_peek_u8(struct reader* nonnull reader,
                        uint8_t* nonnull val,
                        enum edge_mode mode) {
  if (reader->res < 0) {
    return reader->res;
  }

  if (unlikely(reader_avail(reader) < sizeof(uint32_t))) {
    switch (mode) {
      case EDGE_MODE_ERR:
        reader->res = ERR_NO_MEM;
        return ERR_NO_MEM;
      case EDGE_MODE_REFILL:
        return __read_u8_refill(reader, val, true);
      default:
        reader->res = ERR_INVALID_INPUT;
        return ERR_INVALID_INPUT;
    }
  }

  uint32_t next_le = *(uint32_t*)reader->curr;
  if (val != NULL) {
    *val = next_le;
  }

  return OK;
}

static result_t __read_u24_refill(struct reader* nonnull reader,
                                  uint32_t* nonnull val,
                                  bool peek) {
  uint8_t data[4] = {0};

  result_t res = __read_refill(reader, &data, 3, peek);
  if (res < 0) {
    reader->res = res;
    return res;
  }

  uint32_t next_le = *(uint32_t*)data;
  if (val != NULL) {
    *val = be_u24(next_le);
  }

  return OK;
}

result_t reader_read_u24(struct reader* nonnull reader,
                         uint32_t* nonnull val,
                         enum edge_mode mode) {
  if (reader->res < 0) {
    return reader->res;
  }

  if (unlikely(reader_avail(reader) < sizeof(uint32_t))) {
    switch (mode) {
      case EDGE_MODE_ERR:
        reader->res = ERR_NO_MEM;
        return ERR_NO_MEM;
      case EDGE_MODE_REFILL:
        return __read_u24_refill(reader, val, false);
      default:
        reader->res = ERR_INVALID_INPUT;
        return ERR_INVALID_INPUT;
    }
  }

  uint32_t next_le = *(uint32_t*)reader->curr;
  reader->curr += 3;

  if (val != NULL) {
    *val = be_u24(next_le);
  }

  return OK;
}

result_t reader_peek_u24(struct reader* nonnull reader,
                         uint32_t* nonnull val,
                         enum edge_mode mode) {
  if (reader->res < 0) {
    return reader->res;
  }

  if (unlikely(reader_avail(reader) < sizeof(uint32_t))) {
    switch (mode) {
      case EDGE_MODE_ERR:
        reader->res = ERR_NO_MEM;
        return ERR_NO_MEM;
      case EDGE_MODE_REFILL:
        return __read_u24_refill(reader, val, true);
      default:
        reader->res = ERR_INVALID_INPUT;
        return ERR_INVALID_INPUT;
    }
  }

  uint32_t next_le = *(uint32_t*)reader->curr;
  if (val != NULL) {
    *val = be_u24(next_le);
  }

  return OK;
}

static result_t __read_u32_refill(struct reader* nonnull reader,
                                  uint32_t* nonnull val,
                                  bool peek) {
  uint8_t data[4] = {0};

  result_t res = __read_refill(reader, &data, 4, peek);
  if (res < 0) {
    reader->res = res;
    return res;
  }

  uint32_t next_le = *(uint32_t*)data;
  *val = be_u32(next_le);
  return OK;
}

static result_t __peek_u32_refill(struct reader* nonnull reader,
                                  uint32_t* nonnull val) {
  uint8_t data[4] = {0};

  result_t res = __read_refill(reader, &data, 4, true);
  if (res < 0) {
    reader->res = res;
    return res;
  }

  uint32_t next_le = *(uint32_t*)data;
  if (val != NULL) {
    *val = be_u32(next_le);
  }

  return OK_READ;
}

result_t reader_read_u32(struct reader* nonnull reader,
                         uint32_t* nonnull val,
                         enum edge_mode mode) {
  if (reader->res < 0) {
    return reader->res;
  }

  if (unlikely(reader_avail(reader) < sizeof(uint32_t))) {
    switch (mode) {
      case EDGE_MODE_ERR:
        reader->res = ERR_NO_MEM;
        return ERR_NO_MEM;
      case EDGE_MODE_REFILL:
        return __read_u32_refill(reader, val, false);
      default:
        reader->res = ERR_INVALID_INPUT;
        return ERR_INVALID_INPUT;
    }
  }

  uint32_t next_le = be_u32(*(uint32_t*)reader->curr);
  reader->curr += 4;

  if (val != NULL) {
    *val = next_le;
  }

  return OK;
}

result_t reader_peek_u32(struct reader* nonnull reader,
                         uint32_t* nonnull val,
                         enum edge_mode mode) {
  // TODO: Should this have a guard for when reader has an err result?

  if (unlikely(reader_avail(reader) < sizeof(uint32_t))) {
    switch (mode) {
      case EDGE_MODE_ERR:
        reader->res = ERR_NO_MEM;
        return ERR_NO_MEM;
      case EDGE_MODE_REFILL:
        return __peek_u32_refill(reader, val);
      default:
        reader->res = ERR_INVALID_INPUT;
        return ERR_INVALID_INPUT;
    }
  }

  uint32_t next_le = *(uint32_t*)reader->curr;
  if (val != NULL) {
    *val = be_u32(next_le);
  }

  return OK;
}