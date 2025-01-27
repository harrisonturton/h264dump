#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "version.h"
#include "buf.h"
#include "bytes.h"

#define SUCCESS 0
#define FAILURE -1

static const char* USAGE =
    "Usage: h264dump [options] <filename>\n"
    "Options:\n"
    "  -h, --help:    Show this help message and exit\n"
    "  -v, --version: Show the version and exit\n";

int read_nal_units(struct buf* buf) {
  size_t curr = 0;
  printf("Processing bitstream\n");

  if (buf_refill(buf) < 0) {
    fprintf(stderr, "failed to refill bitstream\n");
    return FAILURE;
  }

  uint32_t next;
  size_t overflow;
  for (;;) {
    if (buf_peek_32_be(buf, &next) < 0) {
      fprintf(stderr, "failed to read from bitstream\n");
      return FAILURE;
    }

    if (next == 1) {
      break;
    }

    if (overflow > 2048) {
      fprintf(stderr, "Reached end of start code search without finding it\n");
      return FAILURE;
    }

    if (buf_read_u8(buf, NULL) < 0) {
      fprintf(stderr, "failed to pop u8\n");
      return FAILURE;
    }

    overflow++;
  }

  if (buf_read_u32_be(buf, NULL) < 0) {
    fprintf(stderr, "failed to read from bitstream\n");
    return FAILURE;
  }

  printf("Found starting point\n");
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

  struct buf buf;
  if (buf_file(&buf, argv[1]) != 0) {
    fprintf(stderr, "failed to create file buffer\n");
    return EXIT_FAILURE;
  }

  if (read_nal_units(&buf) < 0) {
    fprintf(stderr, "Failed to read NAL units\n");
    buf_free(&buf);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
