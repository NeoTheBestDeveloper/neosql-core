#pragma once

#include <stdint.h>

// This reader use stream reading.
typedef struct {
    const uint8_t* buf;
    uint64_t buf_size;
    uint64_t buf_offset;
} BufReader;

typedef enum {
    BUF_READER_ok = 0,
    BUF_READER_buffer_overflow = 1,
} BufReaderResultStatus;

typedef struct {
    uint64_t readen;
    BufReaderResultStatus status;
} BufReaderResult;

BufReader buf_reader_new(void const* buf, uint64_t buf_size);
BufReaderResult buf_reader_read(BufReader* reader, void* dst,
                                uint64_t dst_size);
