#include <string.h>

#include "utils/buf_reader.h"

BufReader buf_reader_new(const void *buf, uint64_t buf_size) {
    BufReader reader = {
        .buf = buf,
        .buf_size = buf_size,
        .buf_offset = 0,
    };
    return reader;
}

BufReaderResult buf_reader_read(BufReader *reader, void *dst,
                                uint64_t dst_size) {
    BufReaderResult res;

    if (reader->buf_size - reader->buf_offset >= dst_size) {
        memcpy(dst, reader->buf + reader->buf_offset, dst_size);
        reader->buf_offset += dst_size;

        res.readen = dst_size;
        res.status = BUF_READER_ok;

        return res;
    }

    uint64_t readen = reader->buf_size - reader->buf_offset;
    memcpy(dst, reader->buf, readen);
    reader->buf_offset += readen;

    res.readen = readen;
    res.status = BUF_READER_buffer_overflow;

    return res;
}
