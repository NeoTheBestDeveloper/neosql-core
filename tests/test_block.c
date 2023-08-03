#include <unistd.h>

#include "criterion/assert.h"
#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "database_driver/block.h"
#include "tests/utils.h"
#include "utils/os.h"

#define TABLE1_BLOCK_ADDR ((Addr) { 0, 0 })
#define TABLE1_BLOCK_PAYLOAD_SIZE (55)

#define TABLE2_BLOCK_ADDR ((Addr) { .page_id = 0, .offset = 70 })
#define TABLE2_BLOCK_PAYLOAD_SIZE (63)

#define TABLE1_RECORD1_BLOCK_ADDR ((Addr) { .page_id = 0, .offset = 148 })
#define TABLE1_RECORD1_BLOCK_PAYLOAD_SIZE (21)

#define TABLE2_RECORD1_BLOCK_ADDR ((Addr) { .page_id = 0, .offset = 184 })
#define TABLE2_RECORD1_BLOCK_PAYLOAD_SIZE (30)

#define TABLE2_RECORD2_BLOCK_ADDR ((Addr) { .page_id = 1, .offset = 0 })
#define TABLE2_RECORD2_BLOCK_PAYLOAD_SIZE (5019)

static const BlockHeader table1_block_header_expected = {
    .payload_size = TABLE1_BLOCK_PAYLOAD_SIZE,
    .parted = false,
    .next = { .page_id = 0, .offset = 70 },
};

static const BlockHeader table1_record1_block_header_expected = {
    .payload_size = TABLE1_RECORD1_BLOCK_PAYLOAD_SIZE,
    .parted = false,
    .next = { .page_id = -1, .offset = -1 },
};

static const BlockHeader table2_block_header_expected = {
    .payload_size = TABLE2_BLOCK_PAYLOAD_SIZE,
    .parted = false,
    .next = { .page_id = -1, .offset = -1 },
};

static const BlockHeader table2_record1_block_header_expected = {
    .payload_size = TABLE2_RECORD1_BLOCK_PAYLOAD_SIZE,
    .parted = false,
    .next = { .page_id = 1, .offset = 0 },
};

static const BlockHeader table2_record2_block_header_expected = {
    .payload_size = TABLE2_RECORD2_BLOCK_PAYLOAD_SIZE,
    .parted = true,
    .next = { .page_id = -1, .offset = -1 },
};

static void block_header_assert_eq(const BlockHeader* h1,
                                   const BlockHeader* h2)
{
    cr_assert(eq(u64, h1->payload_size, h2->payload_size));
    cr_assert(addr_eq(h1->next, h2->next));
    cr_assert(h1->parted == h2->parted);
}

Test(TestBlock, test_block_read1)
{
    i32 payload_expected_fd = open_db_file(TABLE1_BLOCK_PAYLOAD_BLOB_PATH);
    u8 payload_expected[TABLE1_BLOCK_PAYLOAD_SIZE];
    read(payload_expected_fd, payload_expected, TABLE1_BLOCK_PAYLOAD_SIZE);
    close(payload_expected_fd);

    i32 fd = open_db_file(TEST_DB_PATH);
    Block block = block_new(TABLE1_BLOCK_ADDR, fd);

    block_header_assert_eq(&block.header, &table1_block_header_expected);
    cr_assert_arr_eq(block.payload, payload_expected,
                     TABLE1_BLOCK_PAYLOAD_SIZE);

    block_free(&block);
    close(fd);
}

Test(TestBlock, test_block_read2)
{
    i32 payload_expected_fd = open_db_file(TABLE2_BLOCK_PAYLOAD_BLOB_PATH);
    u8 payload_expected[TABLE2_BLOCK_PAYLOAD_SIZE];
    read(payload_expected_fd, payload_expected, TABLE2_BLOCK_PAYLOAD_SIZE);
    close(payload_expected_fd);

    i32 fd = open_db_file(TEST_DB_PATH);
    Block block = block_new(TABLE2_BLOCK_ADDR, fd);

    block_header_assert_eq(&block.header, &table2_block_header_expected);
    cr_assert_arr_eq(block.payload, payload_expected,
                     TABLE2_BLOCK_PAYLOAD_SIZE);

    block_free(&block);
    close(fd);
}

Test(TestBlock, test_block_read3)
{
    i32 payload_expected_fd
        = open_db_file(TABLE1_RECORD1_BLOCK_PAYLOAD_BLOB_PATH);
    u8 payload_expected[TABLE1_RECORD1_BLOCK_PAYLOAD_SIZE];
    read(payload_expected_fd, payload_expected,
         TABLE1_RECORD1_BLOCK_PAYLOAD_SIZE);
    close(payload_expected_fd);

    i32 fd = open_db_file(TEST_DB_PATH);
    Block block = block_new(TABLE1_RECORD1_BLOCK_ADDR, fd);

    block_header_assert_eq(&block.header,
                           &table1_record1_block_header_expected);
    cr_assert_arr_eq(block.payload, payload_expected,
                     TABLE1_RECORD1_BLOCK_PAYLOAD_SIZE);

    block_free(&block);
    close(fd);
}

Test(TestBlock, test_block_read4)
{
    i32 payload_expected_fd
        = open_db_file(TABLE2_RECORD1_BLOCK_PAYLOAD_BLOB_PATH);
    u8 payload_expected[TABLE2_RECORD1_BLOCK_PAYLOAD_SIZE];
    read(payload_expected_fd, payload_expected,
         TABLE2_RECORD1_BLOCK_PAYLOAD_SIZE);
    close(payload_expected_fd);

    i32 fd = open_db_file(TEST_DB_PATH);
    Block block = block_new(TABLE2_RECORD1_BLOCK_ADDR, fd);

    block_header_assert_eq(&block.header,
                           &table2_record1_block_header_expected);
    cr_assert_arr_eq(block.payload, payload_expected,
                     TABLE2_RECORD1_BLOCK_PAYLOAD_SIZE);

    block_free(&block);
    close(fd);
}

Test(TestBlock, test_block_read5)
{
    i32 payload_expected_fd
        = open_db_file(TABLE2_RECORD2_BLOCK_PAYLOAD_BLOB_PATH);
    u8 payload_expected[TABLE2_RECORD2_BLOCK_PAYLOAD_SIZE];
    read(payload_expected_fd, payload_expected,
         TABLE2_RECORD2_BLOCK_PAYLOAD_SIZE);
    close(payload_expected_fd);

    i32 fd = open_db_file(TEST_DB_PATH);
    Block block = block_new(TABLE2_RECORD2_BLOCK_ADDR, fd);

    block_header_assert_eq(&block.header,
                           &table2_record2_block_header_expected);
    cr_assert_arr_eq(block.payload, payload_expected,
                     TABLE2_RECORD2_BLOCK_PAYLOAD_SIZE);

    block_free(&block);
    close(fd);
}
