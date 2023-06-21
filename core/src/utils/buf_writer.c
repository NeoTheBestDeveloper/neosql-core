#include <string.h>

#include "buf_writer.h"

BufWriter buf_writer_new(void *buf, u64 buf_size) {
    BufWriter writer = {
        .buf = buf,
        .buf_size = buf_size,
        .buf_offset = 0,
    };

    return writer;
}

BufWriterResult buf_writer_write(BufWriter *writer, const void *payload,
                                 u64 payload_size) {
    BufWriterResult res;

    if (payload_size <= writer->buf_size) {
        memcpy(writer->buf + writer->buf_offset, payload, payload_size);
        writer->buf_offset += payload_size;

        res.written = payload_size;
        res.status = BUF_WRITER_WRITE_OK;
        return res;
    }

    u64 written = writer->buf_size - writer->buf_offset;
    memcpy(writer->buf + writer->buf_offset, payload, written);
    writer->buf_offset += written;

    res.written = written;
    res.status = BUF_WRITER_BUFFER_OVERFLOW;

    return res;
}

const u8 *buf_writer_get_buf(const BufWriter *writer) { return writer->buf; }
