#include "h264/nal_reader.h"
#include <stdint.h>
#include "common/result.h"
#include "reader/reader.h"
#include "test/runtime/runtime.h"

static const char ZERO_BYTE_ANNEXB_FRAME[] =
    "test/h264/data/zero_byte_annexb_frame.bin";
static const char ONE_BYTE_ANNEXB_FRAME[] =
    "test/h264/data/one_byte_annexb_frame.bin";
static const char TWO_BYTE_ANNEXB_FRAME[] =
    "test/h264/data/two_byte_annexb_frame.bin";

static uint8_t ZERO_BYTE_ANNEXB_FRAME_ARR[] = {0, 0, 0, 1, 0, 0, 0, 1};
static uint8_t ONE_BYTE_ANNEXB_FRAME_ARR[] = {0, 0, 0, 1, 0xaa, 0, 0, 0, 1};
static uint8_t TWO_BYTE_ANNEXB_FRAME_ARR[] = {0,    0, 0, 1, 0xaa,
                                              0xbb, 0, 0, 0, 1};
static uint8_t TWO_FRAMES[] = {0, 0, 0, 1, 0xaa, 0, 0, 0, 1, 0xbb, 0, 0, 0, 1};

void test_read_zero_byte_annexb_frame() {
  result_t res;

  struct reader file;
  res = mem_reader(&file, ZERO_BYTE_ANNEXB_FRAME_ARR,
                   sizeof(ZERO_BYTE_ANNEXB_FRAME));
  CHECK(res, "failed to create mem reader: %s\n", res_str(res));

  struct reader nal;
  res = nal_reader(&nal, &file, ANNEXB);
  CHECK(res, "failed to create nal reader: %s\n", res_str(res));

  res = reader_refill(&nal);
  CHECK(res, "failed to refill nal reader: %s\n", res_str(res));

  struct nal_reader* ctx = nal.ctx;
  ASSERT(nal.start == ZERO_BYTE_ANNEXB_FRAME_ARR + 4,
         "unexpected start pointer: %lu\n", nal.start - ctx->src->start);
  ASSERT(nal.end == ZERO_BYTE_ANNEXB_FRAME_ARR + 4,  //
         "unexpected end pointer");
  ASSERT(nal.curr == ZERO_BYTE_ANNEXB_FRAME_ARR + 4,  //
         "unexpected curr pointer");
}

void test_read_one_byte_annexb_frame() {
  result_t res;

  struct reader mem;
  res = mem_reader(&mem, ONE_BYTE_ANNEXB_FRAME_ARR, 9);
  CHECK(res, "failed to create mem reader: %s\n", res_str(res));

  struct reader nal;
  res = nal_reader(&nal, &mem, ANNEXB);
  CHECK(res, "failed to create nal reader: %s\n", res_str(res));

  res = reader_refill(&nal);
  CHECK(res, "failed to refill nal reader: %s\n", res_str(res));

  ASSERT(nal.start == ONE_BYTE_ANNEXB_FRAME_ARR + 4,
         "unexpected start pointer");
  ASSERT(nal.end == ONE_BYTE_ANNEXB_FRAME_ARR + 5, "unexpected end pointer");
  ASSERT(nal.curr == ONE_BYTE_ANNEXB_FRAME_ARR + 4, "unexpected curr pointer");
}

void test_read_two_byte_annexb_frame() {
  result_t res;

  struct reader mem;
  res = mem_reader(&mem, TWO_BYTE_ANNEXB_FRAME_ARR, 10);
  CHECK(res, "failed to create mem reader: %s\n", res_str(res));

  struct reader nal;
  res = nal_reader(&nal, &mem, ANNEXB);
  CHECK(res, "failed to create nal reader: %s\n", res_str(res));

  res = reader_refill(&nal);
  CHECK(res, "failed to refill nal reader: %s\n", res_str(res));

  ASSERT(nal.start == TWO_BYTE_ANNEXB_FRAME_ARR + 4,
         "unexpected start pointer");
  ASSERT(nal.end == TWO_BYTE_ANNEXB_FRAME_ARR + 6, "unexpected end pointer");
  ASSERT(nal.curr == TWO_BYTE_ANNEXB_FRAME_ARR + 4, "unexpected curr pointer");
}

void test_read_two_byte_annexb_frame_across_refill() {
  result_t res;

  struct reader file;
  res = file_reader(&file, TWO_BYTE_ANNEXB_FRAME, 10);
  CHECK(res, "failed to create file reader: %s\n", res_str(res));

  struct reader nal;
  res = nal_reader(&nal, &file, ANNEXB);
  CHECK(res, "failed to create nal reader: %s\n", res_str(res));

  res = reader_refill(&nal);
  CHECK(res, "failed to refill nal reader: %s\n", res_str(res));

  ASSERT(nal.start == TWO_BYTE_ANNEXB_FRAME + 4, "unexpected start pointer");
  ASSERT(nal.end == TWO_BYTE_ANNEXB_FRAME + 6, "unexpected end pointer");
  ASSERT(nal.curr == TWO_BYTE_ANNEXB_FRAME + 4, "unexpected curr pointer");
}

void test_read_two_frames() {
  result_t res;

  struct reader file;
  res = mem_reader(&file, TWO_FRAMES, 14);
  CHECK(res, "failed to create file reader: %s\n", res_str(res));

  struct reader nal;
  res = nal_reader(&nal, &file, ANNEXB);
  CHECK(res, "failed to create nal reader: %s\n", res_str(res));

  res = reader_refill(&nal);
  CHECK(res, "failed to refill nal reader: %s\n", res_str(res));

  ASSERT(nal.start == TWO_FRAMES + 4, "unexpected start pointer");
  ASSERT(nal.end == TWO_FRAMES + 5, "unexpected end pointer");
  ASSERT(nal.curr == TWO_FRAMES + 4, "unexpected curr pointer");

  res = reader_refill(&nal);
  CHECK(res, "failed to refill nal reader: %s\n", res_str(res));

  ASSERT(nal.start == TWO_FRAMES + 9,  //
         "unexpected start pointer after refill");
  ASSERT(nal.end == TWO_FRAMES + 10,  //
         "unexpected end pointer after refill");
  ASSERT(nal.curr == TWO_FRAMES + 9,  //
         "unexpected curr pointer after refill");
}

void test_read_two_frames_across_refill() {
  result_t res;

  struct reader file;
  res = file_reader(&file, TWO_BYTE_ANNEXB_FRAME, 5);
  CHECK(res, "failed to create file reader: %s\n", res_str(res));

  res = reader_refill(&file);
  CHECK(res, "failed to refill file reader: %s\n", res_str(res));

  struct reader nal;
  res = nal_reader(&nal, &file, ANNEXB);
  CHECK(res, "failed to create nal reader: %s\n", res_str(res));

  res = reader_refill(&nal);
  CHECK(res, "failed to refill nal reader: %s\n", res_str(res));

  ASSERT(nal.start == file.start + 4,  //
         "unexpected NAL start, difference: %lu", nal.start - file.start);
  ASSERT(nal.end == file.end,  //
         "unexpected NAL end, difference: %lu", nal.end - file.end);
  ASSERT(reader_curr(&nal) == 0,  //
         "unexpected NAL reader curr: %lu", reader_curr(&nal));
  ASSERT(reader_avail(&nal) == 1,  //
         "uenxpected NAL reader avail: %lu", reader_avail(&nal));

  res = reader_refill(&nal);
  CHECK(res, "failed to refill nal reader: %s\n", res_str(res));

  ASSERT(nal.start == file.start,  //
         "unexpected NAL start after refill, difference: %lu",
         nal.start - file.start);
  ASSERT(nal.end == file.start + 1,  //
         "unexpected NAL end after refill, difference: %lu",
         nal.start - file.end);
  ASSERT(reader_curr(&nal) == 0,  //
         "unexpected NAL reader curr after refill: %lu", reader_curr(&nal));
  ASSERT(reader_avail(&nal) == 1,  //
         "uenxpected NAL reader avail after refill: %lu", reader_avail(&nal));
}

int main(void) {
  RUN(test_read_zero_byte_annexb_frame);
  RUN(test_read_one_byte_annexb_frame);
  RUN(test_read_two_byte_annexb_frame);
  RUN(test_read_two_frames);
  RUN(test_read_two_frames_across_refill);
  return TEST_REPORT();
}