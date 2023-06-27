#pragma once

#include <stdint.h>

// This reader use stream reading.
typedef struct {
    const uint8_t *buf;
    int64_t buf_size;
    int64_t buf_offset;
} BufReader;

typedef enum {
    BUF_READER_ok = 0,
    BUF_READER_buffer_overflow = 1,
} BufReaderResultStatus;

typedef struct {
    int64_t readen;
    BufReaderResultStatus status;
} BufReaderResult;

BufReader buf_reader_new(void const *buf, int64_t buf_size);
BufReaderResult buf_reader_read(BufReader *reader, void *dst, int64_t dst_size);
