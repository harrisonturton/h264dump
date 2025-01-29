#include "buf/buf.h"
#include "buf/mem.h"
#include "error.h"
#include "test.h"

#define INIT_TEST_BUF(buf)               \
  error err = setup_test_buf(buf);       \
  if (err < 0) {                         \
    FAIL("failed to setup test buffer"); \
  }

error setup_test_buf(struct buf* nonnull buf) {
  static uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  return buf_mem(buf, data, sizeof(data));
}

void test_curr_gives_expected_offset_at_start() {
  struct buf buf;
  INIT_TEST_BUF(&buf);

  size_t curr = buf_curr(&buf);
  ASSERT(curr == 0, "unexpected current offset");
}

void test_len_gives_expected_len_at_start() {
  struct buf buf;
  INIT_TEST_BUF(&buf);

  size_t len = buf_len(&buf);
  ASSERT(len == 10, "unexpected buffer length");
}

void test_rem_gives_expected_remainder_at_start() {
  struct buf buf;
  INIT_TEST_BUF(&buf);

  size_t rem = buf_rem(&buf);
  ASSERT(rem == 10, "unexpected buffer remainder");
}

int main(void) {
  RUN(test_curr_gives_expected_offset_at_start);
  RUN(test_len_gives_expected_len_at_start);
  RUN(test_rem_gives_expected_remainder_at_start);
  return TEST_REPORT();
}
