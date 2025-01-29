#ifndef attrs_h
#define attrs_h

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#ifdef __clang__
#define nullable __nullable
#define nonnull __nonnull
#else
#define nullable
#define nonnull
#endif

#endif
