#include "buf/buf.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "common/compiler.h"
#include "common/result.h"
#include "common/endian.h"

result_t buf_refill(struct buf* nonnull buf) {
  return buf->refill(buf);
}

size_t buf_curr(const struct buf* nonnull buf) {
  return buf->curr - buf->start;
}

size_t buf_avail(const struct buf* nonnull buf) {
  return buf->end - buf->curr;
}

size_t buf_len(const struct buf* nonnull buf) {
  return buf->end - buf->start;
}

result_t buf_free(struct buf* nonnull buf) {
  return buf->free(buf);
}

static result_t __read_refill(struct buf* nonnull buf,
                              void* nonnull ptr,
                              size_t len,
                              bool peek) {
  size_t avail = buf_avail(buf);
  size_t missing = len - avail;

  // Copy as many bytes as we can into ptr
  memcpy(ptr, buf->curr, avail);

  // Refill to get the remaining ones
  result_t res = buf_refill(buf);
  if (res < 0) {
    buf->res = res;
    return res;
  }

  // If we still don't have enough data after a refill, give up
  if (buf_avail(buf) < missing) {
    return ERR_NO_MEM;
  }

  // Copy the remaining bytes into ptr
  memcpy(ptr + avail, buf->start, missing);

  if (!peek) {
    buf->curr += missing;
  }

  return OK;
}

static result_t __read_u8_refill(struct buf* nonnull buf,
                                 uint8_t* nonnull val,
                                 bool peek) {
  uint8_t data = 0;

  result_t res = __read_refill(buf, &data, 1, peek);
  if (res < 0) {
    buf->res = res;
    return res;
  }

  *val = data;
  return OK;
}

result_t buf_read_u8(struct buf* nonnull buf,
                     uint8_t* nonnull val,
                     enum edge_mode mode) {
  if (buf->res < 0) {
    return buf->res;
  }

  if (unlikely(buf_avail(buf) < sizeof(uint32_t))) {
    switch (mode) {
      case EDGE_MODE_ERR:
        buf->res = ERR_NO_MEM;
        return ERR_NO_MEM;
      case EDGE_MODE_REFILL:
        return __read_u8_refill(buf, val, false);
      default:
        buf->res = ERR_INVALID_INPUT;
        return ERR_INVALID_INPUT;
    }
  }

  uint8_t next = *(uint8_t*)buf->curr;
  buf->curr += 1;

  if (val != NULL) {
    *val = next;
  }

  return OK;
}

result_t buf_peek_u8(struct buf* nonnull buf,
                     uint8_t* nonnull val,
                     enum edge_mode mode) {
  if (buf->res < 0) {
    return buf->res;
  }

  if (unlikely(buf_avail(buf) < sizeof(uint32_t))) {
    switch (mode) {
      case EDGE_MODE_ERR:
        buf->res = ERR_NO_MEM;
        return ERR_NO_MEM;
      case EDGE_MODE_REFILL:
        return __read_u8_refill(buf, val, true);
      default:
        buf->res = ERR_INVALID_INPUT;
        return ERR_INVALID_INPUT;
    }
  }

  uint32_t next_le = *(uint32_t*)buf->curr;
  buf->curr += 4;

  if (val != NULL) {
    *val = next_le;
  }

  return OK;
}

static result_t __read_u24_refill(struct buf* nonnull buf,
                                  uint32_t* nonnull val,
                                  bool peek) {
  uint8_t data[4] = {0};

  result_t res = __read_refill(buf, &data, 3, peek);
  if (res < 0) {
    buf->res = res;
    return res;
  }

  uint32_t next_le = *(uint32_t*)data;
  *val = be_u24(next_le);
  return OK;
}

result_t buf_read_u24(struct buf* nonnull buf,
                      uint32_t* nonnull val,
                      enum edge_mode mode) {
  if (buf->res < 0) {
    return buf->res;
  }

  if (unlikely(buf_avail(buf) < sizeof(uint32_t))) {
    switch (mode) {
      case EDGE_MODE_ERR:
        buf->res = ERR_NO_MEM;
        return ERR_NO_MEM;
      case EDGE_MODE_REFILL:
        return __read_u24_refill(buf, val, false);
      default:
        buf->res = ERR_INVALID_INPUT;
        return ERR_INVALID_INPUT;
    }
  }

  uint32_t next_le = *(uint32_t*)buf->curr;
  buf->curr += 3;

  if (val != NULL) {
    *val = be_u24(next_le);
  }

  return OK;
}

result_t buf_peek_u24(struct buf* nonnull buf,
                      uint32_t* nonnull val,
                      enum edge_mode mode) {
  if (buf->res < 0) {
    return buf->res;
  }

  if (unlikely(buf_avail(buf) < sizeof(uint32_t))) {
    switch (mode) {
      case EDGE_MODE_ERR:
        buf->res = ERR_NO_MEM;
        return ERR_NO_MEM;
      case EDGE_MODE_REFILL:
        return __read_u24_refill(buf, val, true);
      default:
        buf->res = ERR_INVALID_INPUT;
        return ERR_INVALID_INPUT;
    }
  }

  uint32_t next_le = *(uint32_t*)buf->curr;
  buf->curr += 4;

  if (val != NULL) {
    *val = be_u24(next_le);
  }

  return OK;
}

static result_t __read_u32_refill(struct buf* nonnull buf,
                                  uint32_t* nonnull val,
                                  bool peek) {
  uint8_t data[4] = {0};

  result_t res = __read_refill(buf, &data, 4, peek);
  if (res < 0) {
    buf->res = res;
    return res;
  }

  uint32_t next_le = *(uint32_t*)data;
  *val = be_u32(next_le);
  return OK;
}

static result_t __peek_u32_refill(struct buf* nonnull buf,
                                  uint32_t* nonnull val) {
  uint8_t data[4] = {0};

  result_t res = __read_refill(buf, &data, 4, false);
  if (res < 0) {
    buf->res = res;
    return res;
  }

  uint32_t next_le = *(uint32_t*)data;
  *val = be_u32(next_le);
  return OK;
}

result_t buf_read_u32(struct buf* nonnull buf,
                      uint32_t* nonnull val,
                      enum edge_mode mode) {
  if (buf->res < 0) {
    return buf->res;
  }

  if (unlikely(buf_avail(buf) < sizeof(uint32_t))) {
    switch (mode) {
      case EDGE_MODE_ERR:
        buf->res = ERR_NO_MEM;
        return ERR_NO_MEM;
      case EDGE_MODE_REFILL:
        return __read_u32_refill(buf, val, false);
      default:
        buf->res = ERR_INVALID_INPUT;
        return ERR_INVALID_INPUT;
    }
  }

  uint32_t next_le = be_u32(*(uint32_t*)buf->curr);
  buf->curr += 4;

  if (val != NULL) {
    *val = next_le;
  }

  return OK;
}

result_t buf_peek_u32(struct buf* nonnull buf,
                      uint32_t* nonnull val,
                      enum edge_mode mode) {
  if (buf->res < 0) {
    return buf->res;
  }

  if (unlikely(buf_avail(buf) < sizeof(uint32_t))) {
    switch (mode) {
      case EDGE_MODE_ERR:
        buf->res = ERR_NO_MEM;
        return ERR_NO_MEM;
      case EDGE_MODE_REFILL:
        return __peek_u32_refill(buf, val);
      default:
        buf->res = ERR_INVALID_INPUT;
        return ERR_INVALID_INPUT;
    }
  }

  uint32_t next_le = *(uint32_t*)buf->curr;
  buf->curr += 4;

  if (val != NULL) {
    *val = be_u32(next_le);
  }

  return OK;
}