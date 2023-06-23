#include <fcntl.h>
#include <limits.h>
#include <os.h>
#include <unistd.h>

#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "../../src/db_driver/db_header.h"

typedef enum {
    TEST_HEADER_READ = 0,
    TEST_HEADER_WRITE = 1,
    TEST_HEADER_READ_FAIL_INVALID_MAGIC = 2,
    TEST_HEADER_READ_FAIL_INVALID_HEADER_SIZE = 3,
    TEST_HEADER_READ_FAIL_INVALID_STORAGE_TYPE = 4,
} TestId;

char tmp_files[5][100 + 1] = {
    "tmp_file_test_header_id_0.db", "tmp_file_test_header_id_1.db",
    "tmp_file_test_header_id_2.db", "tmp_file_test_header_id_3.db",
    "tmp_file_test_header_id_4.db"};

void delete_tmp_file(TestId test_id) { unlink(tmp_files[test_id]); }

void _create_valid_header(TestId test_id) {
    i32 fd = open(tmp_files[test_id], O_CREAT | O_WRONLY | O_BINARY, 0666);

    u32 pages_count = 3;
    u8 storage_type = LIST_BLOCKS; // LIST
    u8 reserved[HEADER_RESERVED] = {0};
    Addr first_table = addr_new(0, 50);

    write(fd, NEOSQL_MAGIC, 6);
    write(fd, &pages_count, 4);
    write(fd, &storage_type, 1);
    write(fd, &first_table, 6);
    write(fd, reserved, HEADER_RESERVED);

    close(fd);
}

void create_valid_header(void) { _create_valid_header(TEST_HEADER_READ); }

void create_invalid_magic_header(void) {
    TestId test_id = TEST_HEADER_READ_FAIL_INVALID_MAGIC;
    _create_valid_header(test_id);
    i32 fd = open(tmp_files[test_id], O_WRONLY | O_BINARY, 0666);

    // Corrupt magic
    write(fd, "ABOBA", 5);

    close(fd);
}

void create_invalid_header_size(void) {
    i32 test_id = TEST_HEADER_READ_FAIL_INVALID_HEADER_SIZE;
    i32 fd = open(tmp_files[test_id], O_CREAT | O_RDWR | O_BINARY, 0666);

    u8 reserved[20] = {0};

    write(fd, NEOSQL_MAGIC, 5);
    write(fd, reserved, 20);

    close(fd);
}

void create_invalid_storage_type(void) {
    TestId test_id = TEST_HEADER_READ_FAIL_INVALID_STORAGE_TYPE;
    _create_valid_header(test_id);

    i32 fd = open(tmp_files[test_id], O_WRONLY | O_BINARY, 0666);
    lseek(fd, 10, SEEK_SET);

    u8 storage_type = 10; // Something wrong.

    write(fd, &storage_type, 1);
    close(fd);
}

Test(TestHeader, test_header_read, .init = create_valid_header) {
    i32 test_id = TEST_HEADER_READ;
    i32 fd = open(tmp_files[test_id], O_RDONLY | O_BINARY, 0666);
    DbHeaderResult res = db_header_read(fd);

    cr_assert(eq(u32, res.status, DB_HEADER_OK));

    DbHeader header = res.header;

    cr_assert(eq(u32, header.pages_count, 3));
    cr_assert(eq(u32, header.storage_type, LIST_BLOCKS));
    cr_assert(addr_cmp(header.first_table_addr, addr_new(0, 50)));

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_read_fail_invalid_magic,
     .init = create_invalid_magic_header) {
    TestId test_id = TEST_HEADER_READ_FAIL_INVALID_MAGIC;
    i32 fd = open(tmp_files[test_id], O_RDONLY | O_BINARY);

    DbHeaderResult res = db_header_read(fd);
    cr_assert(eq(u32, res.status, DB_HEADER_INVALID_MAGIC));

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_read_fail_invalid_header_size,
     .init = create_invalid_header_size) {
    TestId test_id = TEST_HEADER_READ_FAIL_INVALID_HEADER_SIZE;
    i32 fd = open(tmp_files[test_id], O_RDONLY | O_BINARY);

    DbHeaderResult res = db_header_read(fd);
    cr_assert(eq(u32, res.status, DB_HEADER_HEADER_SIZE_TOO_SMALL));

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_read_fail_invalid_storage_type,
     .init = create_invalid_storage_type) {
    TestId test_id = TEST_HEADER_READ_FAIL_INVALID_STORAGE_TYPE;
    i32 fd = open(tmp_files[test_id], O_RDONLY | O_BINARY);

    DbHeaderResult res = db_header_read(fd);
    cr_assert(eq(u32, res.status, DB_HEADER_INVALID_STORAGE_TYPE));

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_write) {
    u32 pages_count = 3;
    StorageType storage_type = LIST_BLOCKS;
    Addr first_table = addr_new(0, 60);

    DbHeader header = db_header_new(pages_count, storage_type, first_table);

    TestId test_id = TEST_HEADER_WRITE;
    i32 fd = open(tmp_files[test_id], O_CREAT | O_RDWR | O_BINARY, 0666);
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

    Addr first_table_buf;

    read(fd, &first_table_buf.page_id, 4);
    read(fd, &first_table_buf.offset, 2);
    cr_assert(addr_cmp(first_table_buf, first_table));

    close(fd);
    delete_tmp_file(test_id);
}
