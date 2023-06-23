#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "../../src/db_driver/db_header.h"

typedef enum {
    TEST_HEADER_READ = 0,
    TEST_HEADER_WRITE = 1,
    TEST_HEADER_READ_FAIL_INVALID_MAGIC = 2,
    TEST_HEADER_READ_FAIL_INVALID_PAGE_SIZE = 3,
    TEST_HEADER_READ_FAIL_INVALID_HEADER_SIZE = 4,
    TEST_HEADER_READ_FAIL_INVALID_STORAGE_TYPE = 5,
} TestId;

char tmp_files[6][NAME_MAX + 1] = {
    "tmp_file_test_header_id_0.db", "tmp_file_test_header_id_1.db",
    "tmp_file_test_header_id_2.db", "tmp_file_test_header_id_3.db",
    "tmp_file_test_header_id_4.db", "tmp_file_test_header_id_5.db",
};

void delete_tmp_file(TestId test_id) { unlink(tmp_files[test_id]); }

void _create_valid_header(TestId test_id) {
    i32 fd = open(tmp_files[test_id], O_CREAT | O_RDWR, 0666);

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

void create_valid_header(void) { _create_valid_header(TEST_HEADER_READ); }

void create_invalid_magic_header(void) {
    i32 test_id = TEST_HEADER_READ_FAIL_INVALID_MAGIC;
    _create_valid_header(test_id);
    i32 fd = open(tmp_files[test_id], O_RDWR, 0666);

    // Corrupt magic
    write(fd, "ABOBA", 5);

    close(fd);
}

void create_invalid_header_size(void) {
    i32 test_id = TEST_HEADER_READ_FAIL_INVALID_HEADER_SIZE;
    i32 fd = open(tmp_files[test_id], O_CREAT | O_RDWR, 0666);

    u8 reserved[20] = {0};

    write(fd, NEOSQL_MAGIC, 5);
    write(fd, reserved, 20);

    close(fd);
}

void create_invalid_page_size(void) {
    i32 test_id = TEST_HEADER_READ_FAIL_INVALID_PAGE_SIZE;
    _create_valid_header(test_id);

    i32 fd = open(tmp_files[test_id], O_CREAT | O_RDWR, 0666);
    lseek(fd, 11, SEEK_SET);

    u8 page_size = 10; // something wrong.

    write(fd, &page_size, 1);

    close(fd);
}

void create_invalid_storage_type(void) {
    i32 test_id = TEST_HEADER_READ_FAIL_INVALID_STORAGE_TYPE;
    _create_valid_header(test_id);

    i32 fd = open(tmp_files[test_id], O_CREAT | O_RDWR, 0666);
    lseek(fd, 10, SEEK_SET);

    u8 storage_type = 10; // Something wrong.

    write(fd, &storage_type, 1);
    close(fd);
}

Test(TestHeader, test_header_read, .init = create_valid_header) {
    i32 test_id = TEST_HEADER_READ;
    i32 fd = open(tmp_files[test_id], O_RDWR, 0666);
    DbHeaderResult res = db_header_read(fd);

    cr_expect(res.status == DB_HEADER_OK);

    DbHeader header = res.header;

    cr_expect(header.pages_count == 3);
    cr_expect(header.storage_type == 0);
    cr_expect(header.page_size == 0);
    cr_expect(addr_cmp(header.first_table_node, NullAddr));

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_read_fail_invalid_magic,
     .init = create_invalid_magic_header) {
    i32 test_id = TEST_HEADER_READ_FAIL_INVALID_MAGIC;
    i32 fd = open(tmp_files[test_id], O_RDONLY);

    DbHeaderResult res = db_header_read(fd);
    cr_expect(res.status == DB_HEADER_INVALID_MAGIC);

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_read_fail_invalid_header_size,
     .init = create_invalid_header_size) {
    i32 test_id = TEST_HEADER_READ_FAIL_INVALID_HEADER_SIZE;
    i32 fd = open(tmp_files[test_id], O_RDONLY);

    DbHeaderResult res = db_header_read(fd);
    cr_expect(res.status == DB_HEADER_HEADER_SIZE_TOO_SMALL);

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_read_fail_invalid_page_size,
     .init = create_invalid_page_size) {
    i32 test_id = TEST_HEADER_READ_FAIL_INVALID_PAGE_SIZE;
    i32 fd = open(tmp_files[test_id], O_RDONLY);

    DbHeaderResult res = db_header_read(fd);
    cr_expect(res.status == DB_HEADER_INVALID_PAGE_SIZE);

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_read_fail_invalid_storage_type,
     .init = create_invalid_storage_type) {
    i32 test_id = TEST_HEADER_READ_FAIL_INVALID_STORAGE_TYPE;
    i32 fd = open(tmp_files[test_id], O_RDONLY);

    DbHeaderResult res = db_header_read(fd);
    cr_assert(eq(i32, res.status, DB_HEADER_INVALID_STORAGE_TYPE));

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestHeader, test_header_write) {
    u32 pages_count = 3;
    StorageType storage_type = LIST;
    PageSize page_size = FOUR_KB;
    Addr first_table = addr_new(0, 60, 4096);

    DbHeader header =
        db_header_new(pages_count, storage_type, page_size, first_table);

    i32 test_id = TEST_HEADER_WRITE;
    i32 fd = open(tmp_files[test_id], O_CREAT | O_RDWR, 0666);
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
    delete_tmp_file(test_id);
}
