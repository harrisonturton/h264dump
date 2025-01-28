#ifndef buf_mem_h
#define buf_mem_h

#include "buf.h"
#include "error.h"
#include "mem.h"
#include "attrs.h"

/**
 * Create a buffer pointing to a region of memory.
 * 
 * @param buf pointer to populate with the created buffer.
 * @param mem pointer to the start of the memory region.
 * @param len length of the memory region in bytes.
 */
error buf_mem(struct buf* nonnull buf, void* nonnull ptr, size_t len);

#endif
