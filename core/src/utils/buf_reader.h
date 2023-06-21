#pragma once

#include <types.h>

// This reader use stream reading.
typedef struct {
    const u8 *buf;
    u64 buf_size;
    u64 buf_offset;
} BufReader;

typedef enum {
    BUF_READER_READ_OK = 0,
    BUF_READER_BUFFER_OVERFLOW = 1,
} BufReaderResultStatus;

typedef struct {
    u64 readen;
    BufReaderResultStatus status;
} BufReaderResult;

BufReader buf_reader_new(const void *buf, u64 buf_size);
BufReaderResult buf_reader_read(BufReader *reader, void *dst, u64 dst_size);
