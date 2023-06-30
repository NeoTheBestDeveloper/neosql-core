#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "criterion/assert.h"
#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "driver/addr.h"
#include "driver/defaults.h"
#include "driver/driver.h"
#include "driver/list_block.h"
#include "os.h"
#include "utils/buf_reader.h"

// Static functions from driver.h
ListBlock _driver_read_list_block(Driver* const driver, Addr addr);

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

uint8_t header_reserved_mock[HEADER_RESERVED_SIZE] = { 0 };
uint8_t page_reserved_mock[PAGE_HEADER_RESERVED_SIZE] = { 0 };
uint8_t page_payload_mock[PAGE_PAYLOAD_SIZE] = { 0 };
uint8_t big_payload[10000] = { 0 };

static void delete_tmp_file(TestId test_id) { unlink(tmp_files[test_id]); }

Test(TestDriver, test_driver_db_create)
{
    TestId test_id = TEST_DRIVER_DB_CREATING;
    int32_t fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    Driver driver = driver_create_db(fd);

    lseek(fd, 0, SEEK_SET);

    char magic_buf[6];
    int32_t pages_count_buf;
    int8_t storage_type_buf;
    Addr first_table_buf;
    Addr last_table_buf;
    uint8_t header_reserved_buf[HEADER_RESERVED_SIZE];

    read(fd, magic_buf, 6);
    read(fd, &pages_count_buf, 4);
    read(fd, &storage_type_buf, 1);
    read(fd, &first_table_buf, 6);
    read(fd, &last_table_buf, 6);
    read(fd, header_reserved_buf, HEADER_RESERVED_SIZE);

    cr_assert(0 == strncmp(magic_buf, NEOSQL_MAGIC, 6));
    cr_assert(eq(i32, pages_count_buf, DEFAULT_EMPTY_DB_ZERO_PAGES_COUNT));
    cr_assert(eq(i8, storage_type_buf, STORAGE_TYPE_LIST));
    cr_assert(addr_cmp(first_table_buf, NULL_ADDR));
    cr_assert(addr_cmp(last_table_buf, NULL_ADDR));
    cr_assert_arr_eq(header_reserved_buf, header_reserved_mock,
                     HEADER_RESERVED_SIZE);

    for (int16_t i = 0; i < DEFAULT_EMPTY_DB_ZERO_PAGES_COUNT; ++i) {
        int16_t free_space_buf;
        int16_t first_free_byte_buf;
        uint8_t page_reserved_buf[PAGE_HEADER_RESERVED_SIZE];
        uint8_t page_payload_buf[PAGE_PAYLOAD_SIZE];

        read(fd, &free_space_buf, 2);
        read(fd, &first_free_byte_buf, 2);
        read(fd, page_reserved_buf, PAGE_HEADER_RESERVED_SIZE);
        read(fd, page_payload_buf, PAGE_PAYLOAD_SIZE);

        cr_assert(eq(i16, free_space_buf, PAGE_PAYLOAD_SIZE));
        cr_assert(eq(i16, first_free_byte_buf, 0));
        cr_assert_arr_eq(page_reserved_buf, page_reserved_mock,
                         PAGE_HEADER_RESERVED_SIZE);
        cr_assert_arr_eq(page_payload_buf, page_payload_mock,
                         PAGE_PAYLOAD_SIZE);
    }

    driver_free(&driver);
    delete_tmp_file(test_id);

    close(fd);
}

Test(TestDriver, test_driver_db_open)
{
    TestId test_id = TEST_DRIVER_DB_OPENING;
    int32_t fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    uint32_t page_count = 4;
    StorageType storage_type = STORAGE_TYPE_BTREE;
    Addr first_table = (Addr) { .page_id = 2, .offset = 70 };
    Addr last_table = (Addr) { .page_id = 2, .offset = 700 };

    write(fd, NEOSQL_MAGIC, 6);
    write(fd, &page_count, 4);
    write(fd, &storage_type, 1);
    write(fd, &first_table, 6);
    write(fd, &last_table, 6);
    write(fd, header_reserved_mock, HEADER_RESERVED_SIZE);
    lseek(fd, 0, SEEK_SET);

    DriverResult res = driver_open_db(fd);

    cr_assert(eq(u32, res.status, DRIVER_OK));

    Driver driver = res.driver;
    cr_assert(eq(i32, driver.fd, fd));

    cr_assert(eq(u32, driver.header.pages_count, page_count));
    cr_assert(eq(u32, driver.header.storage_type, storage_type));
    cr_assert(addr_cmp(driver.header.first_table, first_table));
    cr_assert(addr_cmp(driver.header.last_table, last_table));

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestDriver, test_driver_db_invalid_data)
{
    TestId test_id = TEST_DRIVER_DB_OPENING_IVALID_DATA;
    int32_t fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    DriverResult res = driver_open_db(fd);

    cr_assert(eq(u32, res.status, DRIVER_INVALID_OR_CORRUPTED_HEADER));
    lseek(fd, 0, SEEK_SET);

    driver_free(&res.driver);
    delete_tmp_file(test_id);
    close(fd);
}

Test(TestDriver, test_driver_read_block)
{
    int32_t fd = open("../tests/assets/test_database.db",
                      O_CREAT | O_BINARY | O_RDWR, 0666);
    DriverResult res = driver_open_db(fd);

    ListBlock block = _driver_read_list_block(&(res.driver), NULL_ADDR);
    cr_assert_arr_eq(block.payload, "Hi there!", block.header.payload_size);
    cr_assert(block.header.is_overflow == false);
    cr_assert(eq(u64, block.header.payload_size, 10));
    cr_assert(eq(u32, block.header.type, 0));
    cr_assert_arr_eq(&block.header.next, &NULL_ADDR, sizeof NULL_ADDR);

    ListBlock block2 = _driver_read_list_block(
        &(res.driver), (Addr) { .page_id = 1, .offset = 0 });
    cr_assert_arr_eq(block2.payload,
                     "Hi abuses, I'm kind of sort of your king!",
                     block2.header.payload_size);
    cr_assert(block2.header.is_overflow == false);
    cr_assert(eq(u64, block2.header.payload_size, 42));
    cr_assert(eq(u32, block2.header.type, 0));
    cr_assert_arr_eq(&block2.header.next, &NULL_ADDR, sizeof NULL_ADDR);

    list_block_free(&block);
    list_block_free(&block2);

    driver_free(&res.driver);
    close(fd);
}

Test(TestDriver, test_driver_read_overflowed_block)
{
    int32_t fd = open("../tests/assets/test_database.db",
                      O_CREAT | O_BINARY | O_RDWR, 0666);
    DriverResult res = driver_open_db(fd);

    ListBlock block = _driver_read_list_block(
        &(res.driver), (Addr) { .page_id = 0, .offset = 26 });

    for (uint64_t i = 0; i < 10000; ++i) {
        big_payload[i] = i % 255;
    }

    cr_assert_arr_eq(block.payload, big_payload, block.header.payload_size);
    cr_assert(block.header.is_overflow == false);
    cr_assert(eq(u64, block.header.payload_size, 10000));
    cr_assert(eq(u32, block.header.type, 0));
    cr_assert(addr_cmp(block.header.next, NULL_ADDR));

    list_block_free(&block);

    driver_free(&res.driver);
    close(fd);
}
