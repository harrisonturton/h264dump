
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "attrs.h"

#define RUN(func) test_execute((#func), (func))
#define TEST_REPORT() test_report();

#define FAIL(msg)  \
  do {             \
    test_fail(msg); \
  } while (0)

#define ASSERT(expr, msg)   \
  do {                      \
    test_assert(expr, msg); \
    if (!ctx.last_passed) { \
      return;               \
    }                       \
  } while (0)

struct test_ctx {
  bool last_passed;
  char* nullable msg;
  uint32_t failed;
  uint32_t passed;
};

struct test_ctx ctx = {
    .last_passed = false,
    .msg = NULL,
    .failed = 0,
    .passed = 0,
};

void test_init() {
  ctx = (struct test_ctx){
      .last_passed = false,
      .msg = NULL,
      .failed = 0,
      .passed = 0,
  };
}

void test_fail(char* nullable msg) {
  ctx.last_passed = false;
  ctx.msg = msg;
}

void test_assert(bool expr, char* nonnull msg) {
  ctx.msg = expr ? NULL : msg;
  ctx.last_passed = expr;
}

void test_execute(const char* nonnull name, void (*nonnull test_func)()) {
  ctx.last_passed = true;

  test_func();

  if (!ctx.last_passed) {
    fprintf(stderr, "test failed: %s: %s\n", name, ctx.msg);
    ctx.failed++;
    return;
  }

  ctx.passed++;
}

int test_report() {
  fprintf(stderr, "passed: %d, failed: %d\n", ctx.passed, ctx.failed);

  int ret = ctx.failed > 0 ? 1 : 0;
  return ret;
}
