#ifndef result_h
#define result_h

#include "common/compiler.h"

typedef enum {
  OK_FOUND = 1,
  OK = 0,
  ERR_NOT_FOUND = -1,
  ERR_NO_MEM = -2,
  ERR_EOF = -3,
  ERR_INVALID_INPUT = -4,
  ERR_INVALID_STATE = -5,
  ERR_UNIMPLEMENTED = -6,
} result_t;

char* nonnull res_str(result_t res);

#endif