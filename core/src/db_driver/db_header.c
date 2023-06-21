#include <string.h>
#include <unistd.h>

#include "core/src/utils/buf_reader.h"
#include "core/src/utils/buf_writer.h"

#include "db_header.h"

#define KILOBYTE(kb) (kb * 1024)
#define MEGABYTE(mb) (mb * 1024 * 1024)
#define GIGABYTE(gb) (gb * 1024 * 1024 * 1024)

DbHeader db_header_new(u32 pages_count, StorageType storage_type,
                       PageSize page_size, Addr first_table_node) {
    DbHeader header = {
        .pages_count = pages_count,
        .page_size = page_size,
        .storage_type = storage_type,
        .first_table_node = first_table_node,
    };

    return header;
}

DbHeader db_header_new_default(void) {
    return db_header_new(0, LIST, FOUR_KB, NullAddr);
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

    buf_reader_read(&reader, &header.storage_type, 1);
    if (header.storage_type > 1) {
        res.status = DB_HEADER_INVALID_STORAGE_TYPE;
        return res;
    }

    buf_reader_read(&reader, &header.page_size, 1);
    if (header.page_size > 2) {
        res.status = DB_HEADER_INVALID_PAGE_SIZE;
        return res;
    }

    buf_reader_read(&reader, &header.first_table_node.page_number, 4);
    buf_reader_read(&reader, &header.first_table_node.offset, 2);

    res.header = header;
    res.status = DB_HEADER_OK;
    return res;
}

DbHeaderResult db_header_write(const DbHeader *header, i32 fd) {
    u8 header_buf[HEADER_SIZE] = {0};
    BufWriter writer = buf_writer_new(header_buf, HEADER_SIZE);

    buf_writer_write(&writer, NEOSQL_MAGIC, 6);
    buf_writer_write(&writer, &header->pages_count, 4);
    buf_writer_write(&writer, (const u8 *)(&header->storage_type), 1);
    buf_writer_write(&writer, (const u8 *)(&header->page_size), 1);
    buf_writer_write(&writer, &(header->first_table_node.page_number), 4);
    buf_writer_write(&writer, &(header->first_table_node.offset), 2);

    write(fd, buf_writer_get_buf(&writer), HEADER_SIZE);

    DbHeaderResult res = {0};
    res.status = DB_HEADER_OK;

    return res;
}

u64 page_size_to_bytes(PageSize page_size) {
    switch (page_size) {
    case FOUR_KB:
        return KILOBYTE(4);
    case EIGHT_KB:
        return KILOBYTE(8);
    case SIXTEEN_KB:
        return KILOBYTE(16);
    }
}
