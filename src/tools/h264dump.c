#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/endian.h"
#include "common/result.h"
#include "common/log.h"
#include "common/version.h"
#include "buf/buf.h"
#include "tools/args.h"

static const char* USAGE =
    "Usage: h264dump [options] <filename>\n"
    "Options:\n"
    "  -h, --help    Show this help message and exit\n"
    "  -v, --version Show the version and exit\n"
    "  -a, --annexb  Parse bitstream in the annex B format (defaults to avcc)\n";

int main(int argc, char* argv[]) {
  char* path = argv[argc-1];
  bool annexb = false;

  if (argc == 1) {
    log_debug("%s", USAGE);
    return EXIT_SUCCESS;
  }

  if (strncmp(path, "-", 1) == 0) {
    log_debug("Last argument must be the filepath\n");
    return EXIT_SUCCESS;
  }

  if (args_find(argc, argv, "-h") || args_find(argc, argv, "--help")) {
    log_debug("%s", USAGE);
    return EXIT_SUCCESS;
  }

  if (args_find(argc, argv, "-v") || args_find(argc, argv, "--version")) {
    log_debug("v%d.%d\n", VERSION_MAJOR, VERSION_MINOR);
    return EXIT_SUCCESS;
  }

  if (args_find(argc, argv, "-a") || args_find(argc, argv, "--annexb")) {
    annexb = true;
  }

  result_t res;
  uint8_t mem[4] = { 0, 0, 1, 9 };

  printf("Reading with annexb: %d\n", annexb);

  struct buf buf;
  res = buf_mem(&buf, mem, 4);
  if (res < 0) {
    log_debug("failed to create mem buf: %s\n", res_str(res));
    return EXIT_FAILURE;
  }

  printf("rem: %lu\n", buf_avail(&buf));

  uint32_t read;
  res = buf_peek_u32(&buf, &read, EDGE_MODE_ERR);
  if (res < 0) {
    log_debug("failed to read 4 bytes from mem buf: %s\n", res_str(res));
    return EXIT_FAILURE;
  }

  printf("got u32: 0x%08x\n", read);

  return 0;
}