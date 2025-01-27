#ifndef ATTRS_H
#define ATTRS_H

#ifdef __clang__
#define nullable __nullable
#define nonnull __nonnull
#else
#define nullable
#define nonnull
#endif

#endif 
