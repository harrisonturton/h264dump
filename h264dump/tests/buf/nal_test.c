#include "buf/nal.h"
#include "buf/buf.h"
#include "buf/mem.h"
#include "error.h"
#include "test.h"

#define INIT_TEST_BUF(nal_buf, data)           \
  error err = ERR_NONE;                        \
  struct buf mem_buf;                          \
  err = buf_mem(&mem_buf, data, sizeof(data)); \
  if (err < 0) {                               \
    FAIL("failed to setup test mem buffer");   \
  }                                            \
  err = buf_nal(nal_buf, &mem_buf);            \
  if (err < 0) {                               \
    FAIL("failed to setup test NAL buffer");   \
  }

static uint8_t MULTI_BYTE_NALU[11] = {0, 0, 0, 1, 1, 2, 3, 0, 0, 0, 1};
static uint8_t ONE_ZERO_BYTE_NALU[8] = {0, 0, 0, 1, 0, 0, 0, 1};
static uint8_t TWO_ZERO_BYTE_NALUS[12] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1};

void test_nal_buf_refill_handles_one_multi_byte_nalu() {
  struct buf nal_buf;
  INIT_TEST_BUF(&nal_buf, MULTI_BYTE_NALU);

  err = buf_refill(&nal_buf);
  if (err < 0) {
    FAIL("failed to refill NAL buffer");
  }

  uint8_t* nalu = nal_buf.start;
  ASSERT(nalu[0] == 1, "unknown first byte");
  ASSERT(nalu[1] == 2, "unknown second byte");
  ASSERT(nalu[2] == 3, "unknown third byte");
}

void test_nal_buf_refill_handles_two_zero_byte_nalus() {
  struct buf nal_buf;
  INIT_TEST_BUF(&nal_buf, TWO_ZERO_BYTE_NALUS);

  err = buf_refill(&nal_buf);
  if (err < 0) {
    FAIL("failed to refill NAL buffer");
  }

  struct nal_buf_ctx* ctx = buf_ctx(&nal_buf);
  ASSERT(nal_buf.start == TWO_ZERO_BYTE_NALUS+4, "bad start on first refill");
  ASSERT(nal_buf.end == TWO_ZERO_BYTE_NALUS+4, "bad end on first refill");

  err = buf_refill(&nal_buf);
  if (err < 0) {
    FAIL("failed to refill NAL buffer");
  }

  ASSERT(nal_buf.start == TWO_ZERO_BYTE_NALUS+8, "bad start on second refill");
  ASSERT(nal_buf.end == TWO_ZERO_BYTE_NALUS+8, "bad end on second refill");
}

void test_nal_buf_refill_handles_one_zero_byte_nalu() {
  struct buf nal_buf;
  INIT_TEST_BUF(&nal_buf, ONE_ZERO_BYTE_NALU);

  err = buf_refill(&nal_buf);
  if (err < 0) {
    FAIL("failed to refill NAL buffer");
  }

  struct nal_buf_ctx* ctx = buf_ctx(&nal_buf);
  ASSERT(ctx->found_start_code, "refill did not find start code");
  ASSERT(ctx->found_end_code, "refill did not find end code");
  ASSERT(nal_buf.start == ONE_ZERO_BYTE_NALU+4, "bad start");
  ASSERT(nal_buf.end == ONE_ZERO_BYTE_NALU+4, "bad end");

  size_t len = buf_len(&nal_buf);
  ASSERT(len == 0, "unexpected NAL buffer length");
}

int main(void) {
  RUN(test_nal_buf_refill_handles_one_multi_byte_nalu);
  RUN(test_nal_buf_refill_handles_one_zero_byte_nalu);
  return TEST_REPORT();
}
