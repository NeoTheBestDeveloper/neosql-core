#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nclib/stream.h"

#include "database_driver/database_defaults.h"
#include "database_driver/header.h"
#include "utils/pseudo_static.h"
#include "utils/stream_ext.h"

typedef struct {
    HeaderResultStatus status;
    u8 buf[HEADER_SIZE];
} HeaderReadResult;

// PRIVATE METHODS SIGNATURE START.
static inline i64 calc_header_offset();
static void seek_to_header(i32 fd);
static void write_header_buf(const u8* buf, i32 fd);
static HeaderReadResult read_header_buf(i32 fd);
static void header_serialise(const Header* header, u8* buf);
static Header header_deserialise(u8* buf);
// PRIVATE METHODS SIGNATURE END.

// PUBLIC METHODS START.
HeaderResult header_new(i32 fd)
{
    HeaderReadResult res = read_header_buf(fd);

    if (res.status != HEADER_OK) {
        return (HeaderResult) { .status = res.status };
    }

    Header header = header_deserialise(res.buf);

    return (HeaderResult) {
        .header = header,
        .status = HEADER_OK,
    };
}

void header_write(const Header* header, i32 fd)
{
    u8 serialised_header_buf[HEADER_SIZE] = { 0 };
    header_serialise(header, serialised_header_buf);
    write_header_buf(serialised_header_buf, fd);
}
// PUBLIC METHODS END.

static Header header_deserialise(u8* buf)
{
    Stream stream = stream_new(buf, HEADER_SIZE, DEFAULT_DATABASE_ENDIAN);
    return stream_read_header(&stream);
}

// Convert header to bytes which allocated on heap.
static void header_serialise(const Header* header, u8* buf)
{
    Stream stream = stream_new(buf, HEADER_SIZE, DEFAULT_DATABASE_ENDIAN);
    stream_write_header(&stream, header);
}

static inline i64 calc_header_offset() { return 0; }

static void seek_to_header(i32 fd)
{
    lseek(fd, calc_header_offset(), SEEK_SET);
}

static void write_header_buf(const u8* buf, i32 fd)
{
    seek_to_header(fd);
    write(fd, buf, HEADER_SIZE);
}

static HeaderReadResult read_header_buf(i32 fd)
{
    HeaderReadResult res = {
        .status = HEADER_OK,
    };

    seek_to_header(fd);
    i64 readen = read(fd, res.buf, HEADER_SIZE);

    if (readen != 100) {
        return (HeaderReadResult) { .status = HEADER_FILE_INVALID_SIZE };
    }

    if (0 != strncmp(HEADER_MAGIC, (char*)(res.buf), HEADER_MAGIC_SIZE)) {
        return (HeaderReadResult) { .status = HEADER_INVALID_MAGIC };
    }

    return res;
}
