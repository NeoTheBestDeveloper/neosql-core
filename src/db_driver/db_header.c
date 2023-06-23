#include <string.h>
#include <unistd.h>

#include "../utils/buf_reader.h"
#include "../utils/buf_writer.h"

#include "db_header.h"

DbHeader db_header_new_default(void) {
    return db_header_new(1, LIST_BLOCKS, NullAddr, NullAddr);
}

DbHeader db_header_new(u32 pages_count, StorageType storage_type,
                       Addr first_table, Addr last_table) {
    DbHeader header = {
        .pages_count = pages_count,
        .storage_type = storage_type,
        .first_table = first_table,
        .last_table = last_table,
    };

    return header;
}

DbHeaderResult db_header_read(i32 fd) {
    u8 header_buf[HEADER_SIZE] = {0};
    char magic_buf[7] = {0};

    DbHeader header = {0};
    DbHeaderResult res = {0};

    i64 readen = read(fd, header_buf, HEADER_SIZE);

    if (readen < HEADER_SIZE) {
        res.status = DB_HEADER_HEADER_SIZE_TOO_SMALL;
        return res;
    }

    BufReader reader = buf_reader_new(header_buf, HEADER_SIZE);

    // Read magic
    buf_reader_read(&reader, magic_buf, 6);

    if (0 != strncmp(magic_buf, NEOSQL_MAGIC, 6)) {
        res.status = DB_HEADER_INVALID_MAGIC;
        return res;
    }

    buf_reader_read(&reader, &header.pages_count, 4);

    u8 storage_type_buf;
    buf_reader_read(&reader, &storage_type_buf, 1);
    header.storage_type = storage_type_buf;

    if (header.storage_type > 1) {
        res.status = DB_HEADER_INVALID_STORAGE_TYPE;
        return res;
    }

    buf_reader_read(&reader, &header.first_table, 6);
    buf_reader_read(&reader, &header.last_table, 6);

    res.header = header;
    res.status = DB_HEADER_OK;
    return res;
}

DbHeaderResult db_header_write(const DbHeader *header, i32 fd) {
    u8 header_buf[HEADER_SIZE] = {0};
    BufWriter writer = buf_writer_new(header_buf, HEADER_SIZE);

    u8 storage_type = header->storage_type;

    buf_writer_write(&writer, NEOSQL_MAGIC, 6);
    buf_writer_write(&writer, &header->pages_count, 4);
    buf_writer_write(&writer, &storage_type, 1);
    buf_writer_write(&writer, &header->first_table, 6);
    buf_writer_write(&writer, &header->last_table, 6);

    write(fd, buf_writer_get_buf(&writer), HEADER_SIZE);

    DbHeaderResult res = {0};
    res.status = DB_HEADER_OK;

    return res;
}
