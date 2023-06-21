#include <string.h>

#include "buf_reader.h"

BufReader buf_reader_new(const void *buf, u64 buf_size) {
    BufReader reader = {
        .buf = buf,
        .buf_size = buf_size,
        .buf_offset = 0,
    };
    return reader;
}

BufReaderResult buf_reader_read(BufReader *reader, void *dst, u64 dst_size) {
    BufReaderResult res;

    if (reader->buf_size - reader->buf_offset >= dst_size) {
        memcpy(dst, reader->buf + reader->buf_offset, dst_size);
        reader->buf_offset += dst_size;

        res.readen = dst_size;
        res.status = BUF_READER_READ_OK;

        return res;
    }

    u64 readen = reader->buf_size - reader->buf_offset;
    memcpy(dst, reader->buf, readen);
    reader->buf_offset += readen;

    res.readen = readen;
    res.status = BUF_READER_BUFFER_OVERFLOW;

    return res;
}
