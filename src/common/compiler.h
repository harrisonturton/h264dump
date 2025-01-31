#ifndef compiler_h
#define compiler_h

#define nullable __nullable
#define nonnull __nonnull

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif