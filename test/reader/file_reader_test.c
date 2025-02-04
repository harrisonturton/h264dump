#include "common/result.h"
#include "reader/reader.h"
#include "test/runtime/runtime.h"

#define TEST_FILE "test/reader/data/file.bin"

#define INIT_TEST_READER(reader, path, chunk_len)               \
  res = file_reader(reader, path, chunk_len);                   \
  CHECK(res, "failed to create test reader: %s", res_str(res)); \
  res = reader_refill(reader);                                  \
  CHECK(res, "failed to refill test reader: %s", res_str(res));

void test_file_reader_curr_on_new_reader() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader, TEST_FILE, 16);

  size_t curr = reader_curr(&reader);

  ASSERT(curr == 0, "unexpected current index: %zu", curr);
}

void test_file_reader_avail_on_new_reader() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader, TEST_FILE, 16);

  size_t avail = reader_avail(&reader);

  ASSERT(avail == 16, "unexpected available bytes: %zu", avail);
}

void test_file_reader_len_on_new_reader() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader, TEST_FILE, 16);

  size_t len = reader_len(&reader);

  ASSERT(len == 16, "unexpected length: %zu", len);
}

void test_file_reader_read_u8_within_fill() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader, TEST_FILE, 16);

  uint8_t val;
  res = reader_read_u8(&reader, &val, EDGE_MODE_ERR);
  CHECK(res, "reader_read_u8 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x01, "unexpected value: 0x%08d", val);
  ASSERT(reader_curr(&reader) == 1, "unexpected current index: %zu",
         reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 15, "unexpected avail: %zu",
         reader_avail(&reader));
  ASSERT(reader_len(&reader) == 16, "unexpected length: %zu",
         reader_len(&reader));
}

void test_file_reader_read_u8_across_edge_with_edge_mode_err() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader, TEST_FILE, 8);
  reader.curr = reader.end;

  uint8_t val;
  res = reader_read_u8(&reader, &val, EDGE_MODE_ERR);

  ASSERT(res == ERR_NO_MEM, "unexpected result: %s", res_str(res));
  ASSERT(reader_curr(&reader) == 8, "unexpected current index: %zu",
         reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 0, "unexpected avail: %zu",
         reader_avail(&reader));
  ASSERT(reader_len(&reader) == 8, "unexpected length: %zu",
         reader_len(&reader));
}

void test_file_reader_read_u8_across_edge_with_edge_mode_refill() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader, TEST_FILE, 8);
  reader.curr = reader.end;

  uint8_t val;
  res = reader_read_u8(&reader, &val, EDGE_MODE_REFILL);
  CHECK(res, "reader_read_u8 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x09, "unexpected value: 0x%08d", val);
  ASSERT(reader_curr(&reader) == 1, "unexpected current index: %zu",
         reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 7, "unexpected avail: %zu",
         reader_avail(&reader));
  ASSERT(reader_len(&reader) == 8, "unexpected length: %zu",
         reader_len(&reader));
}

void test_file_reader_read_u24_across_edge_with_edge_mode_err() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader, TEST_FILE, 8);
  // Place cursor one byte before end of refill, so u24 attempts to overflow by
  // 2 bytes
  reader.curr = reader.end - 1;

  uint32_t val;
  res = reader_read_u24(&reader, &val, EDGE_MODE_ERR);

  ASSERT(res == ERR_NO_MEM, "unexpected result: %s", res_str(res));
  ASSERT(reader_curr(&reader) == 7, "unexpected current index: %zu",
         reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 1, "unexpected avail: %zu",
         reader_avail(&reader));
  ASSERT(reader_len(&reader) == 8, "unexpected length: %zu",
         reader_len(&reader));
}

void test_file_reader_read_u24_across_edge_with_edge_mode_refill() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader, TEST_FILE, 8);
  // Place cursor one byte before end of refill, so u24 read overflows by 2
  // bytes
  reader.curr = reader.end - 1;

  uint32_t val;
  res = reader_read_u24(&reader, &val, EDGE_MODE_REFILL);
  CHECK(res, "reader_read_u24 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x08090a, "unexpected value: 0x%08d", val);
  ASSERT(reader_curr(&reader) == 2, "unexpected current index: %zu",
         reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 6, "unexpected avail: %zu",
         reader_avail(&reader));
  ASSERT(reader_len(&reader) == 8, "unexpected length: %zu",
         reader_len(&reader));
}

void test_file_reader_read_u32_across_edge_with_edge_mode_err() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader, TEST_FILE, 8);
  // Place cursor 2 byte before end of refill, so u32 read attempts to overflow
  // by 2 bytes
  reader.curr = reader.end - 2;

  uint32_t val;
  res = reader_read_u32(&reader, &val, EDGE_MODE_ERR);

  ASSERT(res == ERR_NO_MEM, "unexpected result: %s", res_str(res));
  ASSERT(reader_curr(&reader) == 6, "unexpected current index: %zu",
         reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 2, "unexpected avail: %zu",
         reader_avail(&reader));
  ASSERT(reader_len(&reader) == 8, "unexpected length: %zu",
         reader_len(&reader));
}

void test_file_reader_read_u32_across_edge_with_edge_mode_refill() {
  result_t res;
  struct reader reader;
  INIT_TEST_READER(&reader, TEST_FILE, 8);
  // Place cursor 2 byte before end of refill, so u32 read overflows by 2 bytes
  reader.curr = reader.end - 2;

  uint32_t val;
  res = reader_read_u32(&reader, &val, EDGE_MODE_REFILL);
  CHECK(res, "reader_read_u32 failed unexpectedly: %s", res_str(res));

  ASSERT(val == 0x0708090a,  //
         "unexpected value: 0x%08d", val);
  ASSERT(reader_curr(&reader) == 2,  //
         "unexpected current index: %zu", reader_curr(&reader));
  ASSERT(reader_avail(&reader) == 6,  //
         "unexpected avail: %zu", reader_avail(&reader));
  ASSERT(reader_len(&reader) == 8,  //
         "unexpected length: %zu", reader_len(&reader));
}

int main(void) {
  RUN(test_file_reader_curr_on_new_reader);
  RUN(test_file_reader_avail_on_new_reader);
  RUN(test_file_reader_len_on_new_reader);
  RUN(test_file_reader_read_u8_within_fill);
  RUN(test_file_reader_read_u8_across_edge_with_edge_mode_err);
  RUN(test_file_reader_read_u8_across_edge_with_edge_mode_refill);
  RUN(test_file_reader_read_u24_across_edge_with_edge_mode_err);
  RUN(test_file_reader_read_u24_across_edge_with_edge_mode_refill);
  RUN(test_file_reader_read_u32_across_edge_with_edge_mode_err);
  RUN(test_file_reader_read_u32_across_edge_with_edge_mode_refill);
  return TEST_REPORT();
}
