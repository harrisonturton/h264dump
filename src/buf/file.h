#ifndef BUF_FILE_H
#define BUF_FILE_H

#include "buf.h"
#include "../error.h"
#include "../attrs.h"

/**
 * Create a new buffer from a file.
 * 
 * @param buf pointer to populate with the created buffer.
 * @param path path of the file to read from.
 */
error buf_file(struct buf* nonnull buf, const char* nonnull path);

#endif
