#include <string.h>

#include "utils/buf_writer.h"

BufWriter buf_writer_new(void* buf, u64 buf_size)
{
    BufWriter writer = {
        .buf = buf,
        .buf_size = buf_size,
        .buf_offset = 0,
    };

    return writer;
}

BufWriterResult buf_writer_write(BufWriter* writer, const void* payload,
                                 u64 payload_size)
{
    BufWriterResult res;
    if (payload_size == 0) {
        res.status = BUF_WRITER_ok;
        res.written = 0;
        return res;
    }

    if (payload_size <= writer->buf_size) {
        memcpy(writer->buf + writer->buf_offset, payload, payload_size);
        writer->buf_offset += payload_size;

        res.written = payload_size;
        res.status = BUF_WRITER_ok;
        return res;
    }

    u64 written = writer->buf_size - writer->buf_offset;
    memcpy(writer->buf + writer->buf_offset, payload, written);
    writer->buf_offset += written;

    res.written = written;
    res.status = BUF_WRITER_buffer_overflow;

    return res;
}

const uint8_t* buf_writer_get_buf(BufWriter writer) { return writer.buf; }
