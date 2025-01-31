#include "buf/buf.h"
#include "common/result.h"
#include "test/runtime/runtime.h"

#define INIT_TEST_BUF(buf)                                  \
  static uint8_t data[10] = {0x01, 0x02, 0x03, 0x04, 0x05,  \
                             0x06, 0x07, 0x08, 0x09, 0x10}; \
  res = buf_mem(buf, data, sizeof(data));                   \
  CHECK(res, "failed to create test buffer: %s", res_str(res));

void test_mem_buf_curr_on_new_buffer() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf);

  size_t curr = buf_curr(&buf);

  ASSERT(curr == 0, "unexpected current index: %zu", curr);
}

void test_mem_buf_avail_on_new_buffer() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf);

  size_t avail = buf_avail(&buf);

  ASSERT(avail == 10, "unexpected available bytes: %zu", avail);
}

void test_mem_buf_len_on_new_buffer() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf);

  size_t len = buf_len(&buf);

  ASSERT(len == 10, "unexpected length: %zu", len);
}

void test_mem_buf_read_u8_within_fill() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf);

  uint8_t val;
  res = buf_read_u8(&buf, &val, EDGE_MODE_ERR);
  CHECK(res, "buf_read_u8 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x01, "unexpected value: 0x%08d", val);
  ASSERT(buf_curr(&buf) == 1, "unexpected current index: %zu", buf_curr(&buf));
  ASSERT(buf_avail(&buf) == 9, "unexpected avail: %zu", buf_avail(&buf));
  ASSERT(buf_len(&buf) == 10, "unexpected length: %zu", buf_len(&buf));
}

void test_mem_buf_read_u24_within_fill() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf);

  uint32_t val;
  res = buf_read_u24(&buf, &val, EDGE_MODE_ERR);
  CHECK(res, "buf_read_u24 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x010203, "unexpected value: 0x%08x", val);
  ASSERT(buf_curr(&buf) == 3, "unexpected current index: %zu", buf_curr(&buf));
  ASSERT(buf_avail(&buf) == 7, "unexpected avail: %zu", buf_avail(&buf));
  ASSERT(buf_len(&buf) == 10, "unexpected length: %zu", buf_len(&buf));
}

void test_mem_buf_read_u32_within_fill() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf);

  uint32_t val;
  res = buf_read_u32(&buf, &val, EDGE_MODE_ERR);
  CHECK(res, "buf_read_u32 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x01020304, "unexpected value: 0x%08x", val);
  ASSERT(buf_curr(&buf) == 4, "unexpected current index: %zu", buf_curr(&buf));
  ASSERT(buf_avail(&buf) == 6, "unexpected avail: %zu", buf_avail(&buf));
  ASSERT(buf_len(&buf) == 10, "unexpected length: %zu", buf_len(&buf));
}

int main(void) {
  RUN(test_mem_buf_curr_on_new_buffer);
  RUN(test_mem_buf_avail_on_new_buffer);
  RUN(test_mem_buf_len_on_new_buffer);
  RUN(test_mem_buf_read_u8_within_fill);
  RUN(test_mem_buf_read_u24_within_fill);
  RUN(test_mem_buf_read_u32_within_fill);
  return TEST_REPORT();
}
