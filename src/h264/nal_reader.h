#ifndef nal_reader_h
#define nal_reader_h

#include <stdbool.h>
#include "common/compiler.h"
#include "common/result.h"
#include "reader/reader.h"

/**
 * NAL bitstream encoding formats.
 */
enum nal_bitstream {
  ANNEXB,
  AVCC,
};

struct avcc_iter {};

struct annexb_iter {
  bool found_start_code;
  bool found_end_code;
};

union nal_iter_state {
  struct avcc_iter avcc;
  struct annexb_iter annexb;
};

/**
 * Context for the NAL unit buffer reader.
 */
struct nal_reader {
  struct reader* nonnull src;
  enum nal_bitstream bitstream;
  union nal_iter_state iter;
};

/**
 * Create a buffer for reading NAL units.
 */
result_t nal_reader(struct reader* nonnull reader,
                    struct reader* nonnull src,
                    enum nal_bitstream bitstream);

#endif