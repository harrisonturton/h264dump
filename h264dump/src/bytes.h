#ifndef BYTES_H
#define BYTES_H

#include <stdint.h>

static inline uint32_t u32_be(uint32_t val) {
  return ((val >> 24) & 0xff)       //
         | ((val << 8) & 0xff0000)  //
         | ((val >> 8) & 0xff00)    //
         | ((val << 24) & 0xff000000);
}

#endif