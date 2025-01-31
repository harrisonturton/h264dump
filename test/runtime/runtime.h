#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "common/compiler.h"

#define RUN(func) test_execute((#func), (func))
#define TEST_REPORT() test_report();

#define CHECK(res, tpl, ...)    \
  do {                          \
    if (res < 0) {              \
      FAIL(tpl, ##__VA_ARGS__); \
    }                           \
  } while (0)

#define ASSERT(expr, tpl, ...)                                          \
  do {                                                                  \
    if (!(expr)) {                                                      \
      snprintf(test_ctx.msg, sizeof(test_ctx.msg), tpl, ##__VA_ARGS__); \
      test_ctx.last_line = __LINE__;                                    \
      test_ctx.last_passed = false;                                     \
      return;                                                           \
    }                                                                   \
  } while (0)

#define FAIL(tpl, ...)                                                \
  do {                                                                \
    snprintf(test_ctx.msg, sizeof(test_ctx.msg), tpl, ##__VA_ARGS__); \
    test_ctx.last_line = __LINE__;                                    \
    test_ctx.last_passed = false;                                     \
    return;                                                           \
  } while (0)

struct test_test_ctx {
  bool last_passed;
  char msg[100];
  int last_line;
  uint32_t failed;
  uint32_t passed;
};

struct test_test_ctx test_ctx = {
    .last_passed = false,
    .msg = {0},
    .failed = 0,
    .passed = 0,
};

void test_init() {
  test_ctx = (struct test_test_ctx){
      .last_passed = false,
      .msg = {0},
      .failed = 0,
      .passed = 0,
  };
}

// void test_fail(char* nullable msg) {
//   test_ctx.last_passed = false;
//   test_ctx.msg = msg;
// }

// void test_assert(bool expr, int line, char* nonnull msg) {
//   test_ctx.msg = expr ? NULL : msg;
//   test_ctx.last_line = line;
//   test_ctx.last_passed = expr;
// }

void test_execute(const char* nonnull name, void (*nonnull test_func)()) {
  test_ctx.last_passed = true;

  test_func();

  if (!test_ctx.last_passed) {
    fprintf(stderr, "test failed: line %d: %s: %s\n",  //
            test_ctx.last_line, name, test_ctx.msg);
    test_ctx.failed++;
    return;
  }

  test_ctx.passed++;
}

int test_report() {
  fprintf(stderr, "passed: %d, failed: %d\n", test_ctx.passed, test_ctx.failed);

  int ret = test_ctx.failed > 0 ? 1 : 0;
  return ret;
}