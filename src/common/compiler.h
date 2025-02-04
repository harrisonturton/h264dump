#ifndef compiler_h
#define compiler_h

#include <stddef.h>

#define nullable __nullable
#define nonnull __nonnull

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define container_of(ptr, type, member)               \
  ({                                                  \
    const typeof(((type*)0)->member)* __mptr = (ptr); \
    (type*)((char*)__mptr - offsetof(type, member));  \
  })

#endif