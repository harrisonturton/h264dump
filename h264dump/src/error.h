#ifndef ERROR_H
#define ERROR_H

typedef enum _error {
  ERR_NONE = 0,
  ERR_NOT_FOUND = -1,
  ERR_NOMEM = -2,
  ERR_EOF = -3,
  ERR_REFILL_FAILED = -4,
  ERR_INVAL = -5,
} error;

#endif
