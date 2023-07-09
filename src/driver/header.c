#include <string.h>
#include <unistd.h>

#include "utils/buf_reader.h"
#include "utils/buf_writer.h"

#include "driver/defaults.h"
#include "driver/header.h"

Header header_default(void)
{
    return (Header) {
        .pages_count = DEFAULT_EMPTY_DB_ZERO_PAGES_COUNT,
        .first_table = NULL_ADDR,
        .last_table = NULL_ADDR,
        .storage_type = DEFAULT_STORAGE_TYPE,
    };
}

Header header_new(StorageType storage_type)
{
    return (Header) {
        .pages_count = DEFAULT_EMPTY_DB_ZERO_PAGES_COUNT,
        .first_table = NULL_ADDR,
        .last_table = NULL_ADDR,
        .storage_type = storage_type,
    };
}

HeaderResult header_read(i32 fd)
{
    u8 header_buf[HEADER_SIZE] = { 0 };
    char magic_buf[7] = { 0 };

    Header header = { 0 };
    HeaderResult res = { 0 };

    lseek(fd, 0, SEEK_SET);
    i64 readen = read(fd, header_buf, HEADER_SIZE);

    // Check header size.
    if (readen < HEADER_SIZE) {
        res.status = HEADER_FILE_SIZE_TOO_SMALL;
        return res;
    }

    BufReader reader = buf_reader_new(header_buf, HEADER_SIZE);

    // Read magic
    buf_reader_read(&reader, magic_buf, 6);

    // Validate readen magic.
    if (0 != strncmp(magic_buf, NEOSQL_MAGIC, 6)) {
        res.status = HEADER_INVALID_MAGIC;
        return res;
    }

    buf_reader_read(&reader, &(header.pages_count), 4);

    i8 storage_type_buf;
    buf_reader_read(&reader, &storage_type_buf, 1);
    header.storage_type = storage_type_buf;

    // Validate storage type.
    if (header.storage_type > 1) {
        res.status = HEADER_INVALID_STORAGE_TYPE;
        return res;
    }

    buf_reader_read(&reader, &(header.first_table), 6);
    buf_reader_read(&reader, &(header.last_table), 6);

    res.header = header;
    res.status = HEADER_OK;
    return res;
}

void header_write(Header const* header, i32 fd)
{
    u8 header_buf[HEADER_SIZE] = { 0 };
    BufWriter writer = buf_writer_new(header_buf, HEADER_SIZE);

    i8 storage_type = header->storage_type;

    buf_writer_write(&writer, NEOSQL_MAGIC, 6);
    buf_writer_write(&writer, &(header->pages_count), 4);
    buf_writer_write(&writer, &storage_type, 1);
    buf_writer_write(&writer, &(header->first_table), 6);
    buf_writer_write(&writer, &(header->last_table), 6);

    lseek(fd, 0, SEEK_SET);
    write(fd, buf_writer_get_buf(writer), HEADER_SIZE);
}
