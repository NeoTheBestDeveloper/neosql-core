#pragma once

#include <types.h>

// This writer use stream writing.
typedef struct {
    u8 *buf;
    u64 buf_offset;
    u64 buf_size;
} BufWriter;

typedef enum {
    BUF_WRITER_WRITE_OK = 0,
    BUF_WRITER_BUFFER_OVERFLOW = 1,
} BufWriterResultStatus;

typedef struct {
    u64 written;
    BufWriterResultStatus status;
} BufWriterResult;

BufWriter buf_writer_new(void *buf, u64 buf_size);
BufWriterResult buf_writer_write(BufWriter *writer, const void *payload,
                                 u64 payload_size);
const u8 *buf_writer_get_buf(const BufWriter *writer);
