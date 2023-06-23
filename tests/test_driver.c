#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <os.h>

#include "criterion/assert.h"
#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "../src/db_driver/db_driver.h"
#include "../src/utils/buf_reader.h"

typedef enum {
    TEST_DRIVER_DB_CREATING = 0,
    TEST_DRIVER_DB_OPENING = 1,
    TEST_DRIVER_DB_OPENING_IVALID_DATA = 2,
} TestId;

char tmp_files[3][100] = {
    "test_tmp_file_test_driver_0.db",
    "test_tmp_file_test_driver_1.db",
    "test_tmp_file_test_driver_2.db",
};

u8 header_reserved_mock[HEADER_RESERVED_SIZE] = {0};
u8 page_reserved_mock[PAGE_HEADER_RESERVED_SIZE] = {0};
u8 page_payload_mock[PAGE_PAYLOAD_SIZE] = {0};

static void delete_tmp_file(TestId test_id) { unlink(tmp_files[test_id]); }

Test(TestDriver, test_driver_db_create) {
    TestId test_id = TEST_DRIVER_DB_CREATING;
    i32 fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    DbDriverResult res = db_driver_create_db(fd);

    cr_assert(eq(u32, res.status, DB_DRIVER_OK));
    lseek(fd, 0, SEEK_SET);

    char magic_buf[6];
    u32 pages_count_buf;
    u8 storage_type_buf;
    Addr first_table_buf;
    u8 header_reserved_buf[HEADER_RESERVED_SIZE];

    read(fd, magic_buf, 6);
    read(fd, &pages_count_buf, 4);
    read(fd, &storage_type_buf, 1);
    read(fd, &first_table_buf, 6);
    read(fd, header_reserved_buf, HEADER_RESERVED_SIZE);

    cr_assert(0 == strncmp(magic_buf, NEOSQL_MAGIC, 6));
    cr_assert(eq(u32, pages_count_buf, 1));
    cr_assert(eq(u8, storage_type_buf, LIST_BLOCKS));
    cr_assert(addr_cmp(first_table_buf, NullAddr));
    cr_assert_arr_eq(header_reserved_buf, header_reserved_mock,
                     HEADER_RESERVED_SIZE);

    u16 free_space_buf;
    u16 first_free_byte_buf;
    u8 page_reserved_buf[PAGE_HEADER_RESERVED_SIZE];
    u8 page_payload_buf[PAGE_PAYLOAD_SIZE];

    read(fd, &free_space_buf, 2);
    read(fd, &first_free_byte_buf, 2);
    read(fd, page_reserved_buf, PAGE_HEADER_RESERVED_SIZE);
    read(fd, page_payload_buf, PAGE_PAYLOAD_SIZE);

    cr_assert(eq(u16, free_space_buf, PAGE_PAYLOAD_SIZE));
    cr_assert(eq(u16, first_free_byte_buf, PAGE_HEADER_SIZE));
    cr_assert_arr_eq(page_reserved_buf, page_reserved_mock,
                     PAGE_HEADER_RESERVED_SIZE);
    cr_assert_arr_eq(page_payload_buf, page_payload_mock, PAGE_PAYLOAD_SIZE);

    db_driver_free(&res.driver);
    delete_tmp_file(test_id);

    close(fd);
}

Test(TestDriver, test_driver_db_open) {
    TestId test_id = TEST_DRIVER_DB_OPENING;
    i32 fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    u32 page_count = 4;
    StorageType storage_type = B_TREE_BLOCKS;
    Addr first_table = addr_new(2, 70);

    write(fd, NEOSQL_MAGIC, 6);
    write(fd, &page_count, 4);
    write(fd, &storage_type, 1);
    write(fd, &first_table, 6);
    write(fd, header_reserved_mock, HEADER_RESERVED_SIZE);
    lseek(fd, 0, SEEK_SET);

    DbDriverResult res = db_driver_open_db(fd);

    cr_assert(eq(u32, res.status, DB_DRIVER_OK));

    DbDriver driver = res.driver;
    cr_assert(eq(i32, driver.db_fd, fd));

    cr_assert(eq(u32, driver.header.pages_count, page_count));
    cr_assert(eq(u32, driver.header.storage_type, storage_type));
    cr_assert(addr_cmp(driver.header.first_table_addr, first_table));

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestDriver, test_driver_db_invalid_data) {
    TestId test_id = TEST_DRIVER_DB_OPENING_IVALID_DATA;
    i32 fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    DbDriverResult res = db_driver_open_db(fd);

    cr_assert(eq(u32, res.status, DB_DRIVER_INVALID_OR_CORRUPTED_HEADER));
    lseek(fd, 0, SEEK_SET);

    db_driver_free(&res.driver);
    delete_tmp_file(test_id);
    close(fd);
}
