#pragma once

#include <stdint.h>

// This writer use stream writing.
typedef struct {
    uint8_t *buf;
    int64_t buf_offset;
    int64_t buf_size;
} BufWriter;

typedef enum {
    BUF_WRITER_ok = 0,
    BUF_WRITER_buffer_overflow = 1,
} BufWriterResultStatus;

typedef struct {
    int64_t written;
    BufWriterResultStatus status;
} BufWriterResult;

BufWriter buf_writer_new(void *buf, int64_t buf_size);

BufWriterResult buf_writer_write(BufWriter *writer, void const *payload,
                                 int64_t payload_size);

uint8_t const *buf_writer_get_buf(BufWriter writer);
