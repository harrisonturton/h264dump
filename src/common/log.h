#ifndef log_h
#define log_h

#include <stdio.h>

#define log_debug(...) fprintf(stdout, __VA_ARGS__)
#define log_err(...) fprintf(stderr, __VA_ARGS__)

#endif