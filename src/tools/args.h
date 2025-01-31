#ifndef args_h
#define args_h

#include <stdbool.h>
#include <string.h>

bool args_find(int argc, char* argv[], char* opt) {
  size_t opt_len = strnlen(opt, 10);

  for (int i = 0; i < argc; i++) {
    if (strncmp(opt, argv[i], opt_len) == 0) {
      return true;
    }
  }

  return false;
}

#endif