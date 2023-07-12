#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "criterion/assert.h"
#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "driver/addr.h"
#include "driver/defaults.h"
#include "driver/driver.h"
#include "driver/list_block.h"
#include "driver/page.h"
#include "table.h"
#include "utils/buf_reader.h"
#include "utils/os.h"

// Static functions from driver.h
ListBlock driver_read_list_block(Driver* const driver, Addr addr);
Addr driver_append_list_block(Driver* driver, const ListBlock* block);
void driver_append_page(Driver* driver, const Page*);

typedef enum {
    TEST_DRIVER_CREATING = 0,
    TEST_DRIVER_OPENING = 1,
    TEST_DRIVER_OPENING_IVALID_DATA = 2,
    TEST_DRIVER_WRITE_BLOCK_AT_EXISTS_PAGE = 3,
    TEST_DRIVER_WRITE_BLOCK_AT_EXISTS_PAGES = 4,
    TEST_DRIVER_APPEND_PAGE = 5,
    TEST_DRIVER_APPEND_TABLE = 6,
    TEST_DRIVER_FIND_TABLE_NOT_FOUND = 7,
    TEST_DRIVER_FIND_TABLE_OK = 8,
} TestId;

char tmp_files[][100] = {
    "test_tmp_file_test_driver_0.db", "test_tmp_file_test_driver_1.db",
    "test_tmp_file_test_driver_2.db", "test_tmp_file_test_driver_3.db",
    "test_tmp_file_test_driver_4.db", "test_tmp_file_test_driver_5.db",
    "test_tmp_file_test_driver_6.db", "test_tmp_file_test_driver_7.db",
    "test_tmp_file_test_driver_8.db",
};

uint8_t header_reserved_mock[HEADER_RESERVED_SIZE] = { 0 };
uint8_t page_reserved_mock[PAGE_HEADER_RESERVED_SIZE] = { 0 };
uint8_t page_payload_mock[PAGE_PAYLOAD_SIZE] = { 0 };
uint8_t big_payload[10000] = { 0 };

static void delete_tmp_file(TestId test_id) { unlink(tmp_files[test_id]); }

static bool table_cmp(const Table* t1, const Table* t2)
{
    bool same = true;

    same = same && t1->records_count == t2->records_count;
    same = same && t1->columns_count == t2->columns_count;

    if (!same) {
        return false;
    }

    same = same && 0 == strcmp(t1->name, t2->name);
    same = same && addr_cmp(t1->first_record, t2->first_record);
    same = same && addr_cmp(t1->last_record, t2->last_record);

    for (u8 i = 0; i < t1->columns_count; ++i) {
        same = same && t1->columns[i].type.id == t2->columns[i].type.id;
        same = same
            && t1->columns[i].type.max_size == t2->columns[i].type.max_size;
        same = same
            && t1->columns[i].type.nullable == t2->columns[i].type.nullable;
        same = same && 0 == strcmp(t1->columns[i].name, t2->columns[i].name);
    }

    return same;
}

Test(TestDriver, test_driver_create)
{
    TestId test_id = TEST_DRIVER_CREATING;
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
    // delete_tmp_file(test_id);

    close(fd);
}

Test(TestDriver, test_driver_open)
{
    TestId test_id = TEST_DRIVER_OPENING;
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

Test(TestDriver, test_driver_invalid_data)
{
    TestId test_id = TEST_DRIVER_OPENING_IVALID_DATA;
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

    Addr zero_addr = { 0, 0 };
    ListBlock block = driver_read_list_block(&(res.driver), zero_addr);
    cr_assert_arr_eq(block.payload, "Hi there!", block.header.payload_size);
    cr_assert(block.header.is_overflow == false);
    cr_assert(eq(u64, block.header.payload_size, 10));
    cr_assert(eq(u32, block.header.type, 0));
    cr_assert_arr_eq(&block.header.next, &NULL_ADDR, 6);

    ListBlock block2 = driver_read_list_block(
        &(res.driver), (Addr) { .page_id = 1, .offset = 0 });
    cr_assert_arr_eq(block2.payload,
                     "Hi abuses, I'm kind of sort of your king!",
                     block2.header.payload_size);
    cr_assert(block2.header.is_overflow == false);
    cr_assert(eq(u64, block2.header.payload_size, 42));
    cr_assert(eq(u32, block2.header.type, 0));
    cr_assert_arr_eq(&block2.header.next, &NULL_ADDR, 6);

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

    ListBlock block = driver_read_list_block(
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

Test(TestDriver, test_driver_write_block_at_exists_page)
{
    TestId test_id = TEST_DRIVER_WRITE_BLOCK_AT_EXISTS_PAGE;
    int32_t fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    Driver driver = driver_create_db(fd);

    ListBlock block = {
        .header = {
            .is_overflow = false,
            .next = (Addr){.offset = 100, .page_id = 0},
            .payload_size = 18,
            .type = LIST_BLOCK_TYPE_RECORD,
        },
        .payload = malloc(18),
    };
    memcpy(block.payload, "HI GUYS I AM PIVO!", block.header.payload_size);

    driver_append_list_block(&driver, &block);

    lseek(fd, 100, SEEK_SET);
    int16_t free_space_buf, first_free_byte_buf;
    read(fd, &free_space_buf, 2);
    read(fd, &first_free_byte_buf, 2);

    cr_assert(eq(i16, first_free_byte_buf,
                 (i16)(block.header.payload_size + LIST_BLOCK_HEADER_SIZE)));
    cr_assert(eq(i16, free_space_buf,
                 (i16)(PAGE_PAYLOAD_SIZE - block.header.payload_size
                       - LIST_BLOCK_HEADER_SIZE)));

    lseek(fd, 100 + PAGE_HEADER_SIZE, SEEK_SET);

    bool is_overflow_buf;
    uint8_t block_type_buf;
    Addr next_buf;
    uint64_t payload_size_buf;
    uint8_t payload_buf[18];

    read(fd, &block_type_buf, 1);
    read(fd, &is_overflow_buf, 1);
    read(fd, &next_buf, 6);
    read(fd, &payload_size_buf, 8);
    read(fd, payload_buf, 18);

    cr_assert(eq(u32, block_type_buf, block.header.type));
    cr_assert(eq(u8, is_overflow_buf, block.header.is_overflow));
    cr_assert_arr_eq(&next_buf, &block.header.next, 6);
    cr_assert(eq(u64, payload_size_buf, block.header.payload_size));
    cr_assert_arr_eq(payload_buf, block.payload, 18);

    list_block_free(&block);
    driver_free(&driver);
    close(fd);

    delete_tmp_file(test_id);
}

Test(TestDriver, test_driver_write_block_at_exists_pages)
{
    TestId test_id = TEST_DRIVER_WRITE_BLOCK_AT_EXISTS_PAGES;
    int32_t fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    Driver driver = driver_create_db(fd);

    lseek(fd, 100, SEEK_SET);
    write(fd, &(int16_t) { 10 }, 2);
    write(fd, &(int16_t) { PAGE_PAYLOAD_SIZE - 10 }, 2);

    ListBlock block = {
        .header = {
            .is_overflow = false,
            .next = (Addr){.offset = 100, .page_id = 0},
            .payload_size = 18,
            .type = LIST_BLOCK_TYPE_RECORD,
        },
        .payload = malloc(18),
    };
    memcpy(block.payload, "HI GUYS I AM PIVO!", block.header.payload_size);

    driver_append_list_block(&driver, &block);

    cr_assert(eq(i32, driver.header.pages_count, 2));

    lseek(fd, 100 + DEFAULT_PAGE_SIZE, SEEK_SET);
    int16_t free_space_buf, first_free_byte_buf;
    read(fd, &free_space_buf, 2);
    read(fd, &first_free_byte_buf, 2);

    cr_assert(
        eq(i16, first_free_byte_buf,
           (int16_t)(block.header.payload_size + LIST_BLOCK_HEADER_SIZE)));
    cr_assert(eq(i16, free_space_buf,
                 (int16_t)(PAGE_PAYLOAD_SIZE - block.header.payload_size
                           - LIST_BLOCK_HEADER_SIZE)));

    list_block_free(&block);
    driver_free(&driver);
    close(fd);

    delete_tmp_file(test_id);
}

Test(TestDriver, test_driver_append_page)
{
    TestId test_id = TEST_DRIVER_WRITE_BLOCK_AT_EXISTS_PAGES;
    int32_t fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    Driver driver = driver_create_db(fd);
    Page page1 = page_new(1);
    Page page2 = page_new(2);

    driver_append_page(&driver, &page1);
    driver_append_page(&driver, &page2);

    cr_assert(eq(i32, driver.header.pages_count, 3));

    int32_t pages_count_buf;

    lseek(fd, 6, SEEK_SET);
    read(fd, &pages_count_buf, 4);
    cr_assert(eq(i32, pages_count_buf, 3));

    lseek(fd, 0, SEEK_SET);
    uint8_t buf[HEADER_SIZE + DEFAULT_PAGE_SIZE * 3];

    int64_t readen = read(fd, buf, HEADER_SIZE + DEFAULT_PAGE_SIZE * 3);
    cr_assert(eq(i64, readen, HEADER_SIZE + DEFAULT_PAGE_SIZE * 3));

    page_free(&page1);
    page_free(&page2);

    driver_free(&driver);
    close(fd);
    delete_tmp_file(test_id);
}

Test(TestDriver, test_driver_append_table)
{
    TestId test_id = TEST_DRIVER_APPEND_TABLE;
    int32_t fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    Driver driver = driver_create_db(fd);

    Column columns1[]
        = { { .name = "c1",
              { .id = TIMESTAMP, .nullable = false, .max_size = 0 } } };

    Column columns2[] = {
        {
            .name = "c1",
            { .id = TIMESTAMP, .nullable = false, .max_size = 0 },
        },
        {
            .name = "c2",
            { .id = VARCHAR, .nullable = false, .max_size = 1024 },
        },
    };

    Table t1 = table_new("Table 1", columns1, 1).table;
    Table t2 = table_new("Table 2", columns2, 1).table;
    Table t3 = table_new("Table 3", columns1, 1).table;

    driver_append_table(&driver, &t1);
    cr_assert(addr_cmp(driver.header.first_table, driver.header.last_table));
    ListBlock block1
        = driver_read_list_block(&driver, driver.header.first_table);
    Table t1_copy = list_block_to_table(&block1);
    cr_assert(table_cmp(&t1_copy, &t1));

    driver_append_table(&driver, &t2);
    ListBlock block2
        = driver_read_list_block(&driver, driver.header.last_table);
    Table t2_copy = list_block_to_table(&block2);
    cr_assert(table_cmp(&t2_copy, &t2));

    driver_append_table(&driver, &t3);
    ListBlock block3
        = driver_read_list_block(&driver, driver.header.last_table);
    Table t3_copy = list_block_to_table(&block3);
    cr_assert(table_cmp(&t3_copy, &t3));

    list_block_free(&block1);
    list_block_free(&block2);
    list_block_free(&block3);

    table_free(&t1);
    table_free(&t1_copy);
    table_free(&t2);
    table_free(&t2_copy);
    table_free(&t3);
    table_free(&t3_copy);

    driver_free(&driver);
    close(fd);
    delete_tmp_file(test_id);
}

Test(TestDriver, test_driver_find_table_not_found)
{
    TestId test_id = TEST_DRIVER_WRITE_BLOCK_AT_EXISTS_PAGES;
    int32_t fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    Driver driver = driver_create_db(fd);

    TableSearchResult res = driver_find_table(&driver, "some name");

    cr_assert(eq(u32, res.status, TABLE_SEARCH_NOT_FOUND));

    driver_free(&driver);
    close(fd);
    delete_tmp_file(test_id);
}

Test(TestDriver, test_driver_find_table_ok)
{
    TestId test_id = TEST_DRIVER_FIND_TABLE_OK;
    int32_t fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    Driver driver = driver_create_db(fd);

    Column columns1[]
        = { { .name = "c1",
              { .id = TIMESTAMP, .nullable = false, .max_size = 0 } } };

    Column columns2[] = {
        {
            .name = "c1",
            { .id = TIMESTAMP, .nullable = false, .max_size = 0 },
        },
        {
            .name = "c2",
            { .id = VARCHAR, .nullable = false, .max_size = 1024 },
        },
    };

    Table t1 = table_new("Table 1", columns1, 1).table;
    Table t2 = table_new("Table 2", columns2, 2).table;
    Table t3 = table_new("Table 3", columns1, 1).table;
    driver_append_table(&driver, &t1);
    driver_append_table(&driver, &t2);
    driver_append_table(&driver, &t3);

    TableSearchResult res1 = driver_find_table(&driver, "Table 2");
    TableSearchResult res2 = driver_find_table(&driver, "Table 1");
    TableSearchResult res3 = driver_find_table(&driver, "Table 3");

    cr_assert(eq(u32, res1.status, TABLE_OK));
    cr_assert(eq(u32, res2.status, TABLE_OK));
    cr_assert(eq(u32, res3.status, TABLE_OK));

    cr_assert(table_cmp(&t1, &(res2.table)));
    cr_assert(table_cmp(&t2, &(res1.table)));
    cr_assert(table_cmp(&t3, &(res3.table)));

    table_free(&t1);
    table_free(&t2);
    table_free(&t3);
    table_free(&(res1.table));
    table_free(&(res2.table));
    table_free(&(res3.table));

    driver_free(&driver);
    close(fd);
    // delete_tmp_file(test_id);
}
