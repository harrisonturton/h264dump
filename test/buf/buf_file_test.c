#include "buf/buf.h"
#include "common/result.h"
#include "test/runtime/runtime.h"

#define TEST_FILE "test/buf/data/file.bin"

#define INIT_TEST_BUF(buf, path, chunk_len)                     \
  res = buf_file(buf, path, chunk_len);                         \
  CHECK(res, "failed to create test buffer: %s", res_str(res)); \
  res = buf_refill(buf);                                        \
  CHECK(res, "failed to refill test buffer: %s", res_str(res));

void test_file_buf_curr_on_new_buffer() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf, TEST_FILE, 16);

  size_t curr = buf_curr(&buf);

  ASSERT(curr == 0, "unexpected current index: %zu", curr);
}

void test_file_buf_avail_on_new_buffer() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf, TEST_FILE, 16);

  size_t avail = buf_avail(&buf);

  ASSERT(avail == 16, "unexpected available bytes: %zu", avail);
}

void test_file_buf_len_on_new_buffer() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf, TEST_FILE, 16);

  size_t len = buf_len(&buf);

  ASSERT(len == 16, "unexpected length: %zu", len);
}

void test_file_buf_read_u8_within_fill() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf, TEST_FILE, 16);

  uint8_t val;
  res = buf_read_u8(&buf, &val, EDGE_MODE_ERR);
  CHECK(res, "buf_read_u8 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x01, "unexpected value: 0x%08d", val);
  ASSERT(buf_curr(&buf) == 1, "unexpected current index: %zu", buf_curr(&buf));
  ASSERT(buf_avail(&buf) == 15, "unexpected avail: %zu", buf_avail(&buf));
  ASSERT(buf_len(&buf) == 16, "unexpected length: %zu", buf_len(&buf));
}

void test_file_buf_read_u8_across_edge_with_edge_mode_err() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf, TEST_FILE, 8);
  buf.curr = buf.end;

  uint8_t val;
  res = buf_read_u8(&buf, &val, EDGE_MODE_ERR);

  ASSERT(res == ERR_NO_MEM, "unexpected result: %s", res_str(res));
  ASSERT(buf_curr(&buf) == 8, "unexpected current index: %zu", buf_curr(&buf));
  ASSERT(buf_avail(&buf) == 0, "unexpected avail: %zu", buf_avail(&buf));
  ASSERT(buf_len(&buf) == 8, "unexpected length: %zu", buf_len(&buf));
}

void test_file_buf_read_u8_across_edge_with_edge_mode_refill() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf, TEST_FILE, 8);
  buf.curr = buf.end;

  uint8_t val;
  res = buf_read_u8(&buf, &val, EDGE_MODE_REFILL);
  CHECK(res, "buf_read_u8 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x09, "unexpected value: 0x%08d", val);
  ASSERT(buf_curr(&buf) == 1, "unexpected current index: %zu", buf_curr(&buf));
  ASSERT(buf_avail(&buf) == 7, "unexpected avail: %zu", buf_avail(&buf));
  ASSERT(buf_len(&buf) == 8, "unexpected length: %zu", buf_len(&buf));
}

void test_file_buf_read_u24_across_edge_with_edge_mode_err() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf, TEST_FILE, 8);
  // Place cursor one byte before end of refill, so u24 attempts to overflow by 2 bytes
  buf.curr = buf.end - 1;

  uint32_t val;
  res = buf_read_u24(&buf, &val, EDGE_MODE_ERR);

  ASSERT(res == ERR_NO_MEM, "unexpected result: %s", res_str(res));
  ASSERT(buf_curr(&buf) == 7, "unexpected current index: %zu", buf_curr(&buf));
  ASSERT(buf_avail(&buf) == 1, "unexpected avail: %zu", buf_avail(&buf));
  ASSERT(buf_len(&buf) == 8, "unexpected length: %zu", buf_len(&buf));
}

void test_file_buf_read_u24_across_edge_with_edge_mode_refill() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf, TEST_FILE, 8);
  // Place cursor one byte before end of refill, so u24 read overflows by 2 bytes 
  buf.curr = buf.end - 1;

  uint32_t val;
  res = buf_read_u24(&buf, &val, EDGE_MODE_REFILL);
  CHECK(res, "buf_read_u24 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x08090a, "unexpected value: 0x%08d", val);
  ASSERT(buf_curr(&buf) == 2, "unexpected current index: %zu", buf_curr(&buf));
  ASSERT(buf_avail(&buf) == 6, "unexpected avail: %zu", buf_avail(&buf));
  ASSERT(buf_len(&buf) == 8, "unexpected length: %zu", buf_len(&buf));
}

void test_file_buf_read_u32_across_edge_with_edge_mode_err() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf, TEST_FILE, 8);
  // Place cursor 2 byte before end of refill, so u32 read attempts to overflow by 2 bytes
  buf.curr = buf.end - 2;

  uint32_t val;
  res = buf_read_u32(&buf, &val, EDGE_MODE_ERR);

  ASSERT(res == ERR_NO_MEM, "unexpected result: %s", res_str(res));
  ASSERT(buf_curr(&buf) == 6, "unexpected current index: %zu", buf_curr(&buf));
  ASSERT(buf_avail(&buf) == 2, "unexpected avail: %zu", buf_avail(&buf));
  ASSERT(buf_len(&buf) == 8, "unexpected length: %zu", buf_len(&buf));
}

void test_file_buf_read_u32_across_edge_with_edge_mode_refill() {
  result_t res;
  struct buf buf;
  INIT_TEST_BUF(&buf, TEST_FILE, 8);
  // Place cursor 2 byte before end of refill, so u32 read overflows by 2 bytes
  buf.curr = buf.end - 2;

  uint32_t val;
  res = buf_read_u32(&buf, &val, EDGE_MODE_REFILL);
  CHECK(res, "buf_read_u32 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x0708090a, "unexpected value: 0x%08d", val);
  ASSERT(buf_curr(&buf) == 2, "unexpected current index: %zu", buf_curr(&buf));
  ASSERT(buf_avail(&buf) == 6, "unexpected avail: %zu", buf_avail(&buf));
  ASSERT(buf_len(&buf) == 8, "unexpected length: %zu", buf_len(&buf));
}

int main(void) {
  RUN(test_file_buf_curr_on_new_buffer);
  RUN(test_file_buf_avail_on_new_buffer);
  RUN(test_file_buf_len_on_new_buffer);
  RUN(test_file_buf_read_u8_within_fill);
  RUN(test_file_buf_read_u8_across_edge_with_edge_mode_err);
  RUN(test_file_buf_read_u8_across_edge_with_edge_mode_refill);
  RUN(test_file_buf_read_u24_across_edge_with_edge_mode_err);
  RUN(test_file_buf_read_u24_across_edge_with_edge_mode_refill);
  RUN(test_file_buf_read_u32_across_edge_with_edge_mode_err);
  RUN(test_file_buf_read_u32_across_edge_with_edge_mode_refill);
  return TEST_REPORT();
}
