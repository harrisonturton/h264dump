#include "common/result.h"
#include "common/compiler.h"

char* nonnull res_str(result_t res) {
  switch (res) {
    case OK_FOUND:
      return "found";
    case OK:
      return "ok";
    case ERR_NOT_FOUND:
      return "not found";
    case ERR_NO_MEM:
      return "no memory";
    case ERR_EOF:
      return "end of file";
    case ERR_INVALID_INPUT:
      return "invalid input";
    case ERR_UNIMPLEMENTED:
      return "unimplemented";
    default:
      return "unknown";
  }
}