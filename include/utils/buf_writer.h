#pragma once

#include "utils/types.h"

// This writer use stream writing.
typedef struct {
    u8* buf;
    u64 buf_offset;
    u64 buf_size;
} BufWriter;

typedef enum {
    BUF_WRITER_ok = 0,
    BUF_WRITER_buffer_overflow = 1,
} BufWriterResultStatus;

typedef struct {
    u64 written;
    BufWriterResultStatus status;
} BufWriterResult;

BufWriter buf_writer_new(void* buf, u64 buf_size);

BufWriterResult buf_writer_write(BufWriter* writer, void const* payload,
                                 u64 payload_size);

uint8_t const* buf_writer_get_buf(BufWriter writer);
