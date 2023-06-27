#include <fcntl.h>
#include <os.h>
#include <unistd.h>

#include "criterion/assert.h"
#include "criterion/criterion.h"
#include "criterion/internal/assert.h"
#include "criterion/internal/new_asserts.h"
#include "criterion/internal/test.h"
#include "criterion/new/assert.h"

#include "../../src/db_driver/list_block.h"
#include "../../src/db_driver/page.h"
#include "../../src/utils/buf_writer.h"

typedef enum {
    TEST_PAGE_READ = 0,
    TEST_PAGE_WRITE = 1,
    TEST_PAGE_READ_TWO_PAGES = 2,
    TEST_PAGE_WRITE_TWO_PAGES = 3,
} TestId;

char tmp_files[5][100 + 1] = {
    "tmp_file_test_page_id_0.db",
    "tmp_file_test_page_id_1.db",
    "tmp_file_test_page_id_2.db",
    "tmp_file_test_page_id_3.db",
};

u8 header_mock[100] = {0};
u8 PAGE_ZERO_PAYLOAD[PAGE_PAYLOAD_SIZE] = {0};

void delete_tmp_file(TestId test_id) { unlink(tmp_files[test_id]); }

void _create_valid_page(TestId test_id, i64 page_offset) {
    for (u8 i = 0; i < 100; ++i) {
        PAGE_ZERO_PAYLOAD[i] = i * (i % 2);
    }
    i32 fd = open(tmp_files[test_id], O_CREAT | O_WRONLY | O_BINARY, 0666);

    u16 free_space = PAGE_PAYLOAD_SIZE;
    u16 first_free_byte = 0;
    u8 reserved[PAGE_HEADER_RESERVED_SIZE] = {0};

    write(fd, header_mock, 100);
    lseek(fd, page_offset, SEEK_CUR);

    write(fd, &free_space, 2);
    write(fd, &first_free_byte, 2);
    write(fd, reserved, PAGE_HEADER_RESERVED_SIZE);
    write(fd, PAGE_ZERO_PAYLOAD, PAGE_PAYLOAD_SIZE);

    close(fd);
}

void create_valid_page(void) { _create_valid_page(TEST_PAGE_READ, 0); }

void create_valid_two_pages(void) {
    _create_valid_page(TEST_PAGE_READ_TWO_PAGES, 0);
    _create_valid_page(TEST_PAGE_READ_TWO_PAGES, DEFAULT_PAGE_SIZE);
}

Test(TestPage, test_page_read, .init = create_valid_page) {
    TestId test_id = TEST_PAGE_READ;

    i32 fd = open(tmp_files[test_id], O_RDONLY | O_BINARY, 0666);

    Page page = page_read(0, fd);

    cr_assert(eq(i32, page.first_free_byte, 0));
    cr_assert(eq(i32, page.free_space, PAGE_PAYLOAD_SIZE));
    cr_assert(eq(i32, page.page_id, 0));

    cr_assert_arr_eq(page.payload, PAGE_ZERO_PAYLOAD, PAGE_PAYLOAD_SIZE);

    page_free(&page);
    close(fd);
    delete_tmp_file(test_id);
}

Test(TestPage, test_page_read_two_pages, .init = create_valid_two_pages) {
    TestId test_id = TEST_PAGE_READ_TWO_PAGES;
    i32 fd = open(tmp_files[test_id], O_RDONLY | O_BINARY, 0666);

    Page page1 = page_read(0, fd);
    cr_assert(eq(i32, page1.first_free_byte, 0));
    cr_assert(eq(i32, page1.free_space, PAGE_PAYLOAD_SIZE));
    cr_assert(eq(i32, page1.page_id, 0));
    cr_assert_arr_eq(page1.payload, PAGE_ZERO_PAYLOAD, PAGE_PAYLOAD_SIZE);

    Page page2 = page_read(1, fd);
    cr_assert(eq(i32, page2.first_free_byte, 0));
    cr_assert(eq(i32, page2.free_space, PAGE_PAYLOAD_SIZE));
    cr_assert(eq(i32, page2.page_id, 1));
    cr_assert_arr_eq(page2.payload, PAGE_ZERO_PAYLOAD, PAGE_PAYLOAD_SIZE);

    page_free(&page1);
    page_free(&page2);

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestPage, test_page_write) {
    TestId test_id = TEST_PAGE_WRITE;
    i32 fd = open(tmp_files[test_id], O_WRONLY | O_CREAT | O_BINARY, 0666);

    write(fd, header_mock, HEADER_SIZE);
    close(fd);

    fd = open(tmp_files[test_id], O_RDWR | O_BINARY, 0666);

    Page page = page_new(0);
    page_write(&page, fd);

    u8 payload_buf[PAGE_PAYLOAD_SIZE];
    u16 free_space_buf;
    u16 first_free_byte_buf;

    lseek(fd, HEADER_SIZE, SEEK_SET);
    read(fd, &free_space_buf, 2);
    read(fd, &first_free_byte_buf, 2);
    read(fd, payload_buf, PAGE_PAYLOAD_SIZE);

    cr_assert(eq(u16, page.free_space, free_space_buf));
    cr_assert(eq(u16, page.first_free_byte, first_free_byte_buf));
    cr_assert_arr_eq(page.payload, payload_buf, PAGE_PAYLOAD_SIZE);

    page_free(&page);

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestPage, test_page_write_two_pages) {
    TestId test_id = TEST_PAGE_WRITE_TWO_PAGES;
    i32 fd = open(tmp_files[test_id], O_WRONLY | O_CREAT | O_BINARY, 0666);

    write(fd, header_mock, HEADER_SIZE);
    close(fd);

    fd = open(tmp_files[test_id], O_RDWR | O_BINARY, 0666);

    Page page1 = page_new(0);
    Page page2 = page_new(1);

    page_write(&page1, fd);
    page_write(&page2, fd);

    for (i32 i = 0; i < 2; ++i) {
        Page page = (i == 0) ? page1 : page2;

        u8 payload_buf[PAGE_PAYLOAD_SIZE];
        u8 reserved[PAGE_HEADER_RESERVED_SIZE];
        u16 free_space_buf;
        u16 first_free_byte_buf;

        lseek(fd, HEADER_SIZE + DEFAULT_PAGE_SIZE * i, SEEK_SET);
        read(fd, &free_space_buf, 2);
        read(fd, &first_free_byte_buf, 2);
        read(fd, reserved, PAGE_HEADER_RESERVED_SIZE);
        read(fd, payload_buf, PAGE_PAYLOAD_SIZE);

        cr_assert(eq(u16, page.free_space, free_space_buf));
        cr_assert(eq(u16, page.first_free_byte, first_free_byte_buf));
        cr_assert_arr_eq(page.payload, payload_buf, PAGE_PAYLOAD_SIZE);
    }

    page_free(&page1);
    page_free(&page2);

    close(fd);
    delete_tmp_file(test_id);
}

Test(TestPage, test_page_read_block) {
    Page page = page_new(0);

    BufWriter writer = buf_writer_new(page.payload, PAGE_PAYLOAD_SIZE);

    ListBlockType block_type = LIST_BLOCK;
    u8 block_type_byte = (u8)block_type;
    bool is_overflow = false;
    Addr next = NullAddr;
    u8 payload[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    u64 payload_size = 10;

    buf_writer_write(&writer, &block_type_byte, 1);
    buf_writer_write(&writer, &is_overflow, 1);
    buf_writer_write(&writer, &next, 6);
    buf_writer_write(&writer, &payload_size, 8);
    buf_writer_write(&writer, payload, payload_size);

    page.first_free_byte = (u16)(1 + 1 + 6 + 8 + payload_size);
    page.free_space = PAGE_PAYLOAD_SIZE - page.first_free_byte;

    ListBlock block = page_read_list_block(&page, 0);

    cr_assert(block.is_overflow == false);
    cr_assert(addr_cmp(block.next, NullAddr));
    cr_assert(eq(u64, block.payload_size, payload_size));
    cr_assert(eq(u32, block.type, block_type));
    cr_assert_arr_eq(block.payload, payload, payload_size);

    list_block_free(&block);
    page_free(&page);
}

Test(TestPage, test_page_append_block) {
    Page page = page_new(0);

    u8 payload[256];
    for (u8 i = 0; i < 255; ++i) {
        payload[i] = i / 2;
    }

    ListBlock block = list_block_new(LIST_BLOCK, payload, 256);
    PageResult res = page_append_list_block(&page, &block);

    cr_assert(eq(u32, res.status, PAGE_OK));
    cr_assert(eq(u64, res.payload_written, 256));

    cr_assert(eq(u16, page.first_free_byte, 256 + LIST_BLOCK_HEADER_SIZE));
    cr_assert(eq(u16, page.free_space,
                 PAGE_PAYLOAD_SIZE - 256 - LIST_BLOCK_HEADER_SIZE));

    cr_assert(eq(u8, page.payload[0], block.type));
    cr_assert(eq(u8, page.payload[1], block.is_overflow));
    cr_assert_arr_eq(page.payload + 2, &block.next, 6);
    cr_assert(eq(u64, *(u64 *)(page.payload + 8), block.payload_size));
    cr_assert_arr_eq(page.payload + LIST_BLOCK_HEADER_SIZE, block.payload, 256);

    list_block_free(&block);
    page_free(&page);
}

Test(TestPage, test_page_append_block_not_enough_place) {
    Page page = page_new(0);
    page.free_space = 10;
    page.first_free_byte = PAGE_PAYLOAD_SIZE - page.free_space;

    u8 payload[256];
    for (u8 i = 0; i < 255; ++i) {
        payload[i] = i / 2;
    }

    ListBlock block = list_block_new(LIST_BLOCK, payload, 256);
    PageResult res = page_append_list_block(&page, &block);

    cr_assert(eq(u32, res.status, PAGE_ERROR_NOT_ENOUGH_SPACE));
    cr_assert(eq(u64, res.payload_written, 0));

    page_free(&page);
}

Test(TestPage, test_page_append_block_with_overflow) {
    Page page = page_new(0);
    page.first_free_byte = 10;
    page.free_space = PAGE_PAYLOAD_SIZE - page.first_free_byte;

    u8 payload[PAGE_PAYLOAD_SIZE + 100];
    for (i32 i = 0; i < PAGE_PAYLOAD_SIZE + 100; ++i) {
        payload[i] = (u8)(i / 2);
    }

    ListBlock block =
        list_block_new(LIST_BLOCK, payload, PAGE_PAYLOAD_SIZE + 100);
    PageResult res = page_append_list_block(&page, &block);

    cr_assert(eq(u32, res.status, PAGE_ERROR_OVERFLOW));
    cr_assert(eq(u64, res.payload_written,
                 PAGE_PAYLOAD_SIZE - 10 - LIST_BLOCK_HEADER_SIZE));

    cr_assert(eq(u16, page.free_space, 0));
    cr_assert(eq(u16, page.first_free_byte, PAGE_PAYLOAD_SIZE));

    cr_assert(eq(u8, page.payload[10], block.type));
    cr_assert(eq(u8, page.payload[10], block.is_overflow));
    cr_assert_arr_eq(page.payload + 12, &block.next, 6);
    cr_assert(eq(u64, *(u64 *)(page.payload + 18), block.payload_size));
    cr_assert_arr_eq(page.payload + LIST_BLOCK_HEADER_SIZE + 10, block.payload,
                     res.payload_written);

    page_free(&page);
}

Test(TestPage, test_page_append_overflowed_block) {
    Page page = page_new(0);
    page.first_free_byte = 10;
    page.free_space = PAGE_PAYLOAD_SIZE - page.first_free_byte;

    u8 payload[PAGE_PAYLOAD_SIZE + 100];
    for (i32 i = 0; i < PAGE_PAYLOAD_SIZE + 100; ++i) {
        payload[i] = (u8)(i / 2);
    }

    ListBlock block =
        list_block_new(LIST_BLOCK, payload, PAGE_PAYLOAD_SIZE + 100);
    PageResult res = page_append_list_block(&page, &block);

    cr_assert(eq(u32, res.status, PAGE_ERROR_OVERFLOW));
    cr_assert(eq(u64, res.payload_written,
                 PAGE_PAYLOAD_SIZE - 10 - LIST_BLOCK_HEADER_SIZE));

    cr_assert(eq(u16, page.free_space, 0));
    cr_assert(eq(u16, page.first_free_byte, PAGE_PAYLOAD_SIZE));

    cr_assert(eq(u8, page.payload[10], block.type));
    cr_assert(eq(u8, page.payload[10], block.is_overflow));
    cr_assert_arr_eq(page.payload + 12, &block.next, 6);
    cr_assert(eq(u64, *(u64 *)(page.payload + 18), block.payload_size));
    cr_assert_arr_eq(page.payload + LIST_BLOCK_HEADER_SIZE + 10, block.payload,
                     res.payload_written);

    page_free(&page);
}
