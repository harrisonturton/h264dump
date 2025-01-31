#include "common/endian.h"
#include <stdint.h>

uint32_t be_u24(uint32_t le) {
  // Need mask to avoid sign extension, though it probably won't happen because
  // this is an unsigned type. But best to be safe.
  return (be_u32(le) >> 8) & 0xffffff;
}

uint32_t be_u32(uint32_t le) {
  return __builtin_bswap32(le);
}