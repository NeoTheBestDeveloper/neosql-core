#include <unistd.h>

#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "nclib.h"

#include "database_driver/block.h"
#include "database_driver/database_defaults.h"
#include "database_driver/header.h"
#include "database_driver/page.h"
#include "tests/utils.h"
#include "utils/os.h"
#include "utils/stream_ext.h"

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

static u8 block_append_not_parted_payload_expected[] = "HI abobuses!!!";
static u8 block_append_not_parted_expected[] = {
    0xe,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x0,  0x48, 0x49, 0x20, 0x61, 0x62,
    0x6f, 0x62, 0x75, 0x73, 0x65, 0x73, 0x21, 0x21, 0x21,
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

// Test(TestBlock, test_block_append_not_parted)
// {
//     Header header = DEFAULT_HEADER;
//     header.pages_count = 1;
//
//     Page page = page_new_zero(0);
//     page.free_space = PAGE_PAYLOAD_SIZE - 100;
//
//     WITH_TMP_FILE(tmp_fd)
//     {
//         header_write(&header, tmp_fd);
//         page_write(&page, tmp_fd);
//
//         Block block = {
//             .payload = block_append_not_parted_payload_expected,
//             .header = {
//                 .payload_size = sizeof
//                 block_append_not_parted_payload_expected, .parted = false,
//                 .next = NULL_ADDR,
//             },
//         };
//
//         block_append(&block, tmp_fd);
//
//         u8 block_buf[sizeof block_append_not_parted_expected];
//         lseek(tmp_fd, HEADER_SIZE + PAGE_HEADER_SIZE + 100, SEEK_SET);
//         read(tmp_fd, block_buf, sizeof block_buf);
//         cr_assert_arr_eq(block_buf, block_append_not_parted_expected,
//                          sizeof block_buf);
//     }
//
//     page_free(&page);
// }
