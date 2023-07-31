#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nclib/stream.h"

#include "database_driver/database_defaults.h"
#include "database_driver/header.h"
#include "utils/pseudo_static.h"
#include "utils/stream_ext.h"

static Header header_deserialise(u8* buf)
{
    Stream stream = stream_new(buf, HEADER_SIZE, DEFAULT_DATABASE_ENDIAN);
    stream_seek(&stream, HEADER_MAGIC_SIZE, STREAM_START);

    Header header;
    header.pages_count = stream_read_u32(&stream);
    header.first_table = stream_read_addr(&stream);
    header.last_table = stream_read_addr(&stream);
    header.cached_pages_count = stream_read_u32(&stream);

    return header;
}

HeaderResult header_new(i32 fd)
{
    lseek(fd, 0, SEEK_SET);
    u8 deserialised_header[HEADER_SIZE] = { 0 };

    i64 readen = read(fd, deserialised_header, HEADER_SIZE);
    if (readen != 100) {
        return (HeaderResult) { .status = HEADER_FILE_INVALID_SIZE };
    }

    if (0
        != strncmp(HEADER_MAGIC, (char*)deserialised_header,
                   HEADER_MAGIC_SIZE)) {
        return (HeaderResult) { .status = HEADER_INVALID_MAGIC };
    }

    Header header = header_deserialise(deserialised_header);

    return (HeaderResult) {
        .header = header,
        .status = HEADER_OK,
    };
}

// Convert header to bytes which allocated on heap.
static u8* header_serialise(const Header* header)
{
    u8* buf = calloc(HEADER_SIZE, 1);

    Stream stream = stream_new(buf, HEADER_SIZE, DEFAULT_DATABASE_ENDIAN);

    stream_write_bytes(&stream, (const u8*)HEADER_MAGIC, HEADER_MAGIC_SIZE);
    stream_write_u32(&stream, header->pages_count);
    stream_write_addr(&stream, header->first_table);
    stream_write_addr(&stream, header->last_table);
    stream_write_u32(&stream, header->cached_pages_count);

    return buf;
}

void header_write(const Header* header, i32 fd)
{
    u8* serialised_header = header_serialise(header);
    lseek(fd, 0, SEEK_SET);
    write(fd, serialised_header, HEADER_SIZE);
    free(serialised_header);
}
