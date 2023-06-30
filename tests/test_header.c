#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "driver/header.h"
#include "os.h"

typedef enum {
    TEST_HEADER_READ = 0,
    TEST_HEADER_WRITE = 1,
    TEST_HEADER_READ_FAIL_INVALID_MAGIC = 2,
    TEST_HEADER_READ_FAIL_INVALID_HEADER_SIZE = 3,
    TEST_HEADER_READ_FAIL_INVALID_STORAGE_TYPE = 4,
} TestId;

char tmp_files[5][100 + 1]
    = { "tmp_file_test_header_id_0.db", "tmp_file_test_header_id_1.db",
        "tmp_file_test_header_id_2.db", "tmp_file_test_header_id_3.db",
        "tmp_file_test_header_id_4.db" };

void delete_tmp_file(TestId test_id) { unlink(tmp_files[test_id]); }

void _create_valid_header(TestId test_id)
{
    int32_t fd = open(tmp_files[test_id], O_CREAT | O_WRONLY | O_BINARY, 0666);

    int32_t pages_count = 3;
    uint8_t storage_type = STORAGE_TYPE_LIST; // LIST
    uint8_t reserved[HEADER_RESERVED_SIZE] = { 0 };
    Addr first_table = (Addr) { .page_id = 0, .offset = 50 };
    Addr last_table = (Addr) { .page_id = 0, .offset = 500 };

    write(fd, NEOSQL_MAGIC, 6);
    write(fd, &pages_count, 4);
    write(fd, &storage_type, 1);
    write(fd, &first_table, 6);
    write(fd, &last_table, 6);
    write(fd, reserved, HEADER_RESERVED_SIZE);

    close(fd);
}

void create_valid_header(void) { _create_valid_header(TEST_HEADER_READ); }

void create_invalid_magic_header(void)
{
    TestId test_id = TEST_HEADER_READ_FAIL_INVALID_MAGIC;
    _create_valid_header(test_id);
    int32_t fd = open(tmp_files[test_id], O_WRONLY | O_BINARY, 0666);

    // Corrupt magic
    write(fd, "ABOBA", 5);

    close(fd);
}

void create_invalid_header_size(void)
{
    int32_t test_id = TEST_HEADER_READ_FAIL_INVALID_HEADER_SIZE;
    int32_t fd = open(tmp_files[test_id], O_CREAT | O_RDWR | O_BINARY, 0666);

    uint8_t reserved[20] = { 0 };

    write(fd, NEOSQL_MAGIC, 5);
    write(fd, reserved, 20);

    close(fd);
}

void create_invalid_storage_type(void)
{
    TestId test_id = TEST_HEADER_READ_FAIL_INVALID_STORAGE_TYPE;
    _create_valid_header(test_id);

    int32_t fd = open(tmp_files[test_id], O_WRONLY | O_BINARY, 0666);
    lseek(fd, 10, SEEK_SET);

    uint8_t storage_type = 10; // Something wrong.

    write(fd, &storage_type, 1);
    close(fd);
}

Test(TestHeader, test_header_read, .init = create_valid_header)
{
    int32_t test_id = TEST_HEADER_READ;
    int32_t fd = open(tmp_files[test_id], O_RDONLY | O_BINARY, 0666);
    HeaderResult res = header_read(fd);

    cr_assert(eq(u32, res.status, HEADER_OK));

    Header header = res.header;

    cr_assert(eq(i32, header.pages_count, 3));
    cr_assert(eq(u32, header.storage_type, STORAGE_TYPE_LIST));
    cr_assert(addr_cmp(header.first_table, (Addr) { 0, 50 }));
    cr_assert(addr_cmp(header.last_table, (Addr) { 0, 500 }));

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_read_fail_invalid_magic,
     .init = create_invalid_magic_header)
{
    TestId test_id = TEST_HEADER_READ_FAIL_INVALID_MAGIC;
    int32_t fd = open(tmp_files[test_id], O_RDONLY | O_BINARY);

    HeaderResult res = header_read(fd);
    cr_assert(eq(u32, res.status, HEADER_INVALID_MAGIC));

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_read_fail_invalid_header_size,
     .init = create_invalid_header_size)
{
    TestId test_id = TEST_HEADER_READ_FAIL_INVALID_HEADER_SIZE;
    int32_t fd = open(tmp_files[test_id], O_RDONLY | O_BINARY);

    HeaderResult res = header_read(fd);
    cr_assert(eq(u32, res.status, HEADER_FILE_SIZE_TOO_SMALL));

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_read_fail_invalid_storage_type,
     .init = create_invalid_storage_type)
{
    TestId test_id = TEST_HEADER_READ_FAIL_INVALID_STORAGE_TYPE;
    int32_t fd = open(tmp_files[test_id], O_RDONLY | O_BINARY);

    HeaderResult res = header_read(fd);
    cr_assert(eq(u32, res.status, HEADER_INVALID_STORAGE_TYPE));

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_write)
{
    int32_t pages_count = 3;
    StorageType storage_type = STORAGE_TYPE_LIST;
    Addr first_table = (Addr) { .page_id = 0, .offset = 60 };
    Addr last_table = (Addr) { .page_id = 0, .offset = 600 };

    Header header = {
        .pages_count = pages_count,
        .first_table = first_table,
        .last_table = last_table,
        .storage_type = storage_type,
    };

    TestId test_id = TEST_HEADER_WRITE;
    int32_t fd = open(tmp_files[test_id], O_CREAT | O_RDWR | O_BINARY, 0666);
    header_write(&header, fd);

    lseek(fd, 0, SEEK_SET);

    char magic_buf[7] = { 0 };
    read(fd, magic_buf, 6);
    cr_assert(eq(str, magic_buf, NEOSQL_MAGIC));

    int32_t pages_count_buf = 0;
    read(fd, &pages_count_buf, 4);
    cr_assert(eq(i32, pages_count_buf, pages_count));

    StorageType storage_type_buf = 0;
    read(fd, &storage_type_buf, 1);
    cr_assert(eq(u32, storage_type, storage_type_buf));

    Addr first_table_buf;
    Addr last_table_buf;

    read(fd, &first_table_buf, 6);
    read(fd, &last_table_buf, 6);
    cr_assert(addr_cmp(first_table_buf, first_table));
    cr_assert(addr_cmp(last_table_buf, last_table));

    close(fd);
    delete_tmp_file(test_id);
}
