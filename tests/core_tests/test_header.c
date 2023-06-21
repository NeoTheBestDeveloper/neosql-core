// TODO: Make tests for independent by using tmp files with random names.

#include <fcntl.h>
#include <unistd.h>

#include "criterion/criterion.h"
#include "criterion/internal/new_asserts.h"
#include "criterion/new/assert.h"

#include "../../core/src/db_driver/db_header.h"

#define TEMP_DB_PATH ("TEMP_DB.db")

void create_valid_header(void) {
    i32 fd = open(TEMP_DB_PATH, O_CREAT | O_RDWR, 0666);

    u32 pages_count = 3;
    u8 storage_type = 0; // LIST
    u8 page_size = 0;    // 4kb
    u8 first_table_node_bytes[6] = {0};
    u8 reserved[82] = {0};

    write(fd, NEOSQL_MAGIC, 6);
    write(fd, &pages_count, 4);
    write(fd, &storage_type, 1);
    write(fd, &page_size, 1);
    write(fd, first_table_node_bytes, 6);
    write(fd, reserved, 82);

    close(fd);
}

void create_invalid_magic_header(void) {
    i32 fd = open(TEMP_DB_PATH, O_CREAT | O_RDWR, 0666);

    u8 reserved[HEADER_SIZE - 5] = {0};

    write(fd, NEOSQL_MAGIC, 5);
    write(fd, reserved, HEADER_SIZE - 5);

    close(fd);
}

void create_invalid_header_size(void) {
    i32 fd = open(TEMP_DB_PATH, O_CREAT | O_RDWR, 0666);

    u8 reserved[20] = {0};

    write(fd, NEOSQL_MAGIC, 5);
    write(fd, reserved, 20);

    close(fd);
}

void create_invalid_page_size(void) {
    i32 fd = open(TEMP_DB_PATH, O_CREAT | O_RDWR, 0666);

    u32 pages_count = 3;
    u8 storage_type = 0; // LIST
    u8 page_size = 10;   // something wrong.
    u8 first_table_node_bytes[6] = {0};
    u8 reserved[82] = {0};

    write(fd, NEOSQL_MAGIC, 6);
    write(fd, &pages_count, 4);
    write(fd, &storage_type, 1);
    write(fd, &page_size, 1);
    write(fd, first_table_node_bytes, 6);
    write(fd, reserved, 82);

    close(fd);
}

void create_invalid_storage_type(void) {
    i32 fd = open(TEMP_DB_PATH, O_CREAT | O_RDWR, 0666);

    u32 pages_count = 3;
    u8 storage_type = 10; // Something wrong.
    u8 page_size = 0;     // 4kb
    u8 first_table_node_bytes[6] = {0};
    u8 reserved[82] = {0};

    write(fd, NEOSQL_MAGIC, 6);
    write(fd, &pages_count, 4);
    write(fd, &storage_type, 1);
    write(fd, &page_size, 1);
    write(fd, first_table_node_bytes, 6);
    write(fd, reserved, 82);

    close(fd);
}

void delete_db_file(void) { unlink(TEMP_DB_PATH); }

Test(suite, test_header_read, .init = create_valid_header,
     .fini = delete_db_file) {
    i32 fd = open(TEMP_DB_PATH, O_RDONLY);
    DbHeaderResult res = db_header_read(fd);

    cr_expect(res.status == DB_HEADER_OK);

    DbHeader header = res.header;

    cr_expect(header.pages_count == 3);
    cr_expect(header.storage_type == 0);
    cr_expect(header.page_size == 0);
    cr_expect(addr_cmp(header.first_table_node, NullAddr));

    close(fd);
}

Test(suite, test_header_read_fail_invalid_magic,
     .init = create_invalid_magic_header, .fini = delete_db_file) {
    i32 fd = open(TEMP_DB_PATH, O_RDONLY);
    DbHeaderResult res = db_header_read(fd);

    cr_expect(res.status == DB_HEADER_INVALID_MAGIC);

    close(fd);
}

Test(suite, test_header_read_fail_invalid_header_size,
     .init = create_invalid_header_size, .fini = delete_db_file) {
    i32 fd = open(TEMP_DB_PATH, O_RDONLY);
    DbHeaderResult res = db_header_read(fd);

    cr_expect(res.status == DB_HEADER_HEADER_SIZE_TOO_SMALL);

    close(fd);
}

Test(suite, test_header_read_fail_invalid_page_size,
     .init = create_invalid_page_size, .fini = delete_db_file) {
    i32 fd = open(TEMP_DB_PATH, O_RDONLY);
    DbHeaderResult res = db_header_read(fd);

    cr_expect(res.status == DB_HEADER_INVALID_PAGE_SIZE);

    close(fd);
}

Test(suite, test_header_read_fail_invalid_storage_type,
     .init = create_invalid_storage_type, .fini = delete_db_file) {
    i32 fd = open(TEMP_DB_PATH, O_RDONLY);
    DbHeaderResult res = db_header_read(fd);

    cr_expect(res.status == DB_HEADER_INVALID_STORAGE_TYPE);

    close(fd);
}

Test(suite, test_header_write, .fini = delete_db_file) {
    u32 pages_count = 3;
    StorageType storage_type = LIST;
    PageSize page_size = FOUR_KB;
    Addr first_table = addr_new(0, 60, 4096);

    DbHeader header =
        db_header_new(pages_count, storage_type, page_size, first_table);

    i32 fd = open(TEMP_DB_PATH, O_CREAT | O_RDWR, 0666);
    DbHeaderResult res = db_header_write(&header, fd);

    cr_expect(res.status == DB_HEADER_OK);

    lseek(fd, 0, SEEK_SET);

    char magic_buf[7] = {0};
    read(fd, magic_buf, 6);
    cr_assert(eq(str, magic_buf, NEOSQL_MAGIC));

    u32 pages_count_buf = 0;
    read(fd, &pages_count_buf, 4);
    cr_assert(eq(u32, pages_count_buf, pages_count));

    StorageType storage_type_buf = 0;
    read(fd, &storage_type_buf, 1);
    cr_assert(eq(u32, storage_type, storage_type_buf));

    PageSize page_size_buf = 0;
    read(fd, &page_size_buf, 1);
    cr_assert(eq(u32, page_size_buf, page_size));

    Addr first_table_buf = {
        .page_size = page_size_to_bytes(page_size_buf),
    };

    read(fd, &first_table_buf.page_number, 4);
    read(fd, &first_table_buf.offset, 2);
    cr_assert(addr_cmp(first_table_buf, first_table));

    close(fd);
}
