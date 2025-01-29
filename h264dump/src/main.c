#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buf/buf.h"
#include "buf/file.h"
#include "buf/nal.h"
#include "bytes.h"
#include "version.h"

static const char* USAGE =
    "Usage: h264dump [options] <filename>\n"
    "Options:\n"
    "  -h, --help:    Show this help message and exit\n"
    "  -v, --version: Show the version and exit\n";

error find_nal_len(struct buf* nal_buf, size_t* nonnull ret) {
  error err = ERR_NONE;
  size_t len = 0;

  struct nal_buf_ctx* ctx = buf_ctx(nal_buf);

  for (;;) {
    if (!ctx->found_start_code || !ctx->found_end_code) {
      printf("Refilling, found start: %d, found end: %d\n",
             ctx->found_start_code, ctx->found_end_code);

      err = buf_refill(nal_buf);
      if (err < 0) {
        fprintf(stderr, "initial NAL refill failed\n");
        return err;
      }
    }

    len += buf_len(nal_buf);

    if (ctx->found_end_code) {
      break;
    }
  }

  *ret = len;
  return ERR_NONE;
}

error read_nal_unit(struct buf* src) {
  error err = ERR_NONE;

  void* start = 0;
  void* end = 0;

  for (;;) {
    uint32_t next_four_bytes;
    uint32_t next_three_bytes;

    err = buf_peek_u32_be(src, &next_four_bytes);
    if (err < 0) {
      return err;
    }

    err = buf_peek_u24_be(src, &next_three_bytes);
    if (err < 0) {
      return err;
    }

    if (next_four_bytes == 0x1 || next_three_bytes == 0x1) {
      break;
    }

    err = buf_read_u8(src, NULL);
    if (err < 0) {
      return err;
    }
  }

  uint32_t next_three_bytes;

  err = buf_peek_u24_be(src, &next_three_bytes);
  if (err < 0) {
    return err;
  }

  if (next_three_bytes != 0x1) {
    err = buf_read_u8(src, NULL);
    if (err < 0) {
      return err;
    }
  }

  // Pop start code prefix
  src->curr += 3;
  start = src->curr;

  for (;;) {
    uint32_t next_four_bytes;
    uint32_t next_three_bytes;

    err = buf_peek_u32_be(src, &next_four_bytes);
    if (err < 0) {
      if (err == ERR_EOF) {
        end = src->end;
        goto end;
      }

      return err;
    }

    err = buf_peek_u24_be(src, &next_three_bytes);
    if (err < 0) {
      return err;
    }

    if (next_four_bytes == 0x1 || next_three_bytes == 0x1) {
      break;
    }

    err = buf_read_u8(src, NULL);
    if (err < 0) {
      return err;
    }
  }

  end = src->curr;

end:
  printf("found nal with length: %lu\n", end - start);

  // while( more_data_in_byte_stream( ) && next_bits( 24 ) != 0x000001 && next_bits( 32 ) != 0x00000001 )

  // size_t offset = 0;

  // for (;;) {
  //   uint32_t next_four_bytes;
  //   uint32_t next_three_bytes;

  //   err = buf_peek_u32_be(src, &next_four_bytes);
  //   if (err < 0) {
  //     return err;
  //   }

  //   next_three_bytes = (next_four_bytes >> 8) & 0xffffff;

  //   if (next_four_bytes == 0 || next_four_bytes == 1) {
  //     printf("offset: 0x%08zx\n", offset);
  //   }

  //   offset++;
  //   src->curr++;
  // }

  // struct buf nal_buf;
  // err = buf_nal(&nal_buf, src);
  // if (err < 0) {
  //   return err;
  // }

  // for (int i = 0; i < 10; i++) {
  //   size_t len;

  //   err = buf_refill(&nal_buf);
  //   if (err < 0) {
  //     if (err == ERR_EOF) {
  //       printf("EOF reached\n");
  //       return 0;
  //     }

  //     fprintf(stderr, "NAL refill failed\n");
  //     return err;
  //   }

  //   // err = find_nal_len(&nal_buf, &len);
  //   // if (err < 0) {
  //   //   fprintf(stderr, "failed to find NAL length\n");
  //   //   return err;
  //   // }

  //   // printf("Found NAL with length: %lu\n", len);
  //   printf("current length: %lu\n", buf_len(&nal_buf));
  //   printf("-------------------------------\n");
  //   getchar();
  // }

  return SUCCESS;
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    printf("%s", USAGE);
    return EXIT_FAILURE;
  }

  if (strncmp("-h", argv[1], 2) == 0 || strncmp("--help", argv[0], 6) == 0) {
    printf("%s", USAGE);
    return EXIT_SUCCESS;
  }

  if (strncmp("-v", argv[1], 2) == 0 || strncmp("--version", argv[0], 9) == 0) {
    printf("v%d.%d\n", VERSION_MAJOR, VERSION_MINOR);
    return EXIT_SUCCESS;
  }

  struct buf file_buf;
  if (buf_file(&file_buf, argv[1]) != 0) {
    fprintf(stderr, "failed to create file buffer\n");
    return EXIT_FAILURE;
  }

  for (int i = 0; i < 51; i++) {
    error err = read_nal_unit(&file_buf);
    if (err < 0) {
      if (err == ERR_EOF) {
        printf("reached end\n");
        return EXIT_SUCCESS;
      }

      fprintf(stderr, "Failed to read NAL unit\n");
      buf_free(&file_buf);
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

// size_t curr = 0;
// printf("Processing bitstream\n");

// if (buf_refill(buf) < 0) {
//   fprintf(stderr, "failed to refill bitstream\n");
//   return FAILURE;
// }

// size_t len = 0;
// size_t offset = 0;

// uint32_t next;
// size_t overflow;
// for (;;) {
//   if (buf_peek_u32_be(buf, &next) < 0) {
//     fprintf(stderr, "failed to read from bitstream\n");
//     return FAILURE;
//   }

//   if (next == 1) {
//     break;
//   }

//   if (overflow > 2048) {
//     fprintf(stderr, "Reached end of start code search without finding it\n");
//     return FAILURE;
//   }

//   if (buf_read_u8(buf, NULL) < 0) {
//     fprintf(stderr, "failed to pop u8\n");
//     return FAILURE;
//   }

//   offset += 1;
//   overflow++;
// }

// if (buf_read_u32_be(buf, NULL) < 0) {
//   fprintf(stderr, "failed to read from bitstream\n");
//   return FAILURE;
// }
// offset += 4;

// printf("Found starting point\n");

// overflow = 0;
// for (;;) {
//   uint32_t next;
//   if (buf_read_u32_be(buf, &next) < 0) {
//     fprintf(stderr, "failed to read from bitstream\n");
//     return FAILURE;
//   }

//   offset += 4;

//   if ((next & 0x00ffffff) == 1) {
//     printf("Found nal length %lu at offset: %lu based on value 0x1\n", len,
//     offset); return EXIT_SUCCESS;
//   }

//   if ((next & 0x00ffffff) == 0) {
//     printf("Found nal length %lu at offset: %lu based on value 0x0\n", len,
//     offset); return EXIT_SUCCESS;
//   }

//   len += 4;

//   if (overflow > 2048) {
//     printf("Did not find end point within search range\n");
//     return EXIT_FAILURE;
//   }

//   overflow++;
// }
