#include "common/result.h"
#include "reader/reader.h"
#include "test/runtime/runtime.h"

#define INIT_TEST_READER(reader)                            \
  static uint8_t data[10] = {0x01, 0x02, 0x03, 0x04, 0x05,  \
                             0x06, 0x07, 0x08, 0x09, 0x10}; \
  res = mem_reader(reader, data, sizeof(data));             \
  CHECK(res, "failed to create test reader: %s", res_str(res));

void test_mem_reader_curr_on_new_reader() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader);

  size_t curr = reader_curr(&reader);

  ASSERT(curr == 0, "unexpected current index: %zu", curr);
}

void test_mem_reader_avail_on_new_reader() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader);

  size_t avail = reader_avail(&reader);

  ASSERT(avail == 10, "unexpected available bytes: %zu", avail);
}

void test_mem_reader_len_on_new_reader() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader);

  size_t len = reader_len(&reader);

  ASSERT(len == 10, "unexpected length: %zu", len);
}

void test_mem_reader_read_u8_within_fill() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader);

  uint8_t val;
  res = reader_read_u8(&reader, &val, EDGE_MODE_ERR);
  CHECK(res, "reader_read_u8 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x01, "unexpected value: 0x%08d", val);
  ASSERT(reader_curr(&reader) == 1, "unexpected current index: %zu",
         reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 9, "unexpected avail: %zu",
         reader_avail(&reader));
  ASSERT(reader_len(&reader) == 10, "unexpected length: %zu",
         reader_len(&reader));
}

void test_mem_reader_peek_u8_within_fill() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader);

  uint8_t val;
  res = reader_peek_u8(&reader, &val, EDGE_MODE_ERR);
  CHECK(res, "reader_read_u8 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x01,  //
         "unexpected value: 0x%08d", val);
  ASSERT(reader_curr(&reader) == 0,  //
         "unexpected current index: %zu", reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 10,  //
         "unexpected avail: %zu", reader_avail(&reader));
  ASSERT(reader_len(&reader) == 10,  //
         "unexpected length: %zu", reader_len(&reader));
}

void test_mem_reader_read_u24_within_fill() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader);

  uint32_t val;
  res = reader_read_u24(&reader, &val, EDGE_MODE_ERR);
  CHECK(res, "reader_read_u24 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x010203, "unexpected value: 0x%08x", val);
  ASSERT(reader_curr(&reader) == 3, "unexpected current index: %zu",
         reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 7, "unexpected avail: %zu",
         reader_avail(&reader));
  ASSERT(reader_len(&reader) == 10, "unexpected length: %zu",
         reader_len(&reader));
}

void test_mem_reader_peek_u24_within_fill() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader);

  uint32_t val;
  res = reader_peek_u24(&reader, &val, EDGE_MODE_ERR);
  CHECK(res, "reader_read_u24 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x010203,  //
         "unexpected value: 0x%08x", val);
  ASSERT(reader_curr(&reader) == 0,  //
         "unexpected current index: %zu", reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 10,  //
         "unexpected avail: %zu", reader_avail(&reader));
  ASSERT(reader_len(&reader) == 10,  //
         "unexpected length: %zu", reader_len(&reader));
}

void test_mem_reader_read_u32_within_fill() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader);

  uint32_t val;
  res = reader_read_u32(&reader, &val, EDGE_MODE_ERR);
  CHECK(res, "reader_read_u32 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x01020304, "unexpected value: 0x%08x", val);
  ASSERT(reader_curr(&reader) == 4, "unexpected current index: %zu",
         reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 6, "unexpected avail: %zu",
         reader_avail(&reader));
  ASSERT(reader_len(&reader) == 10, "unexpected length: %zu",
         reader_len(&reader));
}

void test_mem_reader_peek_u32_within_fill() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader);

  uint32_t val;
  res = reader_peek_u32(&reader, &val, EDGE_MODE_ERR);
  CHECK(res, "reader_read_u32 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x01020304,  //
         "unexpected value: 0x%08x", val);
  ASSERT(reader_curr(&reader) == 0,  //
         "unexpected current index: %zu", reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 10,  //
         "unexpected avail: %zu", reader_avail(&reader));
  ASSERT(reader_len(&reader) == 10,  //
         "unexpected length: %zu", reader_len(&reader));
}

int main(void) {
  RUN(test_mem_reader_curr_on_new_reader);
  RUN(test_mem_reader_avail_on_new_reader);
  RUN(test_mem_reader_len_on_new_reader);
  RUN(test_mem_reader_read_u8_within_fill);
  RUN(test_mem_reader_peek_u8_within_fill);
  RUN(test_mem_reader_read_u24_within_fill);
  RUN(test_mem_reader_peek_u24_within_fill);
  RUN(test_mem_reader_read_u32_within_fill);
  RUN(test_mem_reader_peek_u32_within_fill);
  return TEST_REPORT();
}
