#include <fcntl.h>
#include <os.h>
#include <unistd.h>

#include "criterion/assert.h"
#include "criterion/criterion.h"
#include "criterion/internal/assert.h"
#include "criterion/internal/test.h"
#include "criterion/new/assert.h"

#include "../../src/db_driver/page.h"

typedef enum {
    TEST_PAGE_READ = 0,
    TEST_PAGE_WRITE = 1,
    TEST_PAGE_READ_TWO_PAGES = 2,
    TEST_PAGE_WRITE_TWO_PAGES = 3,
} TestId;

char tmp_files[4][100 + 1] = {
    "tmp_file_test_page_id_0.db",
    "tmp_file_test_page_id_1.db",
    "tmp_file_test_page_id_2.db",
    "tmp_file_test_page_id_3.db",
};

u8 header_mock[100] = {0};
u8 PAGE_ZERO_PAYLOAD[PAGE_PAYLOAD_SIZE] = {0};

void delete_tmp_file(TestId test_id) { unlink(tmp_files[test_id]); }

void _create_valid_page(TestId test_id, i64 page_offset) {
    for (i32 i = 0; i < 100; ++i) {
        PAGE_ZERO_PAYLOAD[i] = i * (i % 2);
    }
    i32 fd = open(tmp_files[test_id], O_CREAT | O_WRONLY | O_BINARY, 0666);

    u16 free_space = PAGE_PAYLOAD_SIZE;
    u16 first_free_byte = PAGE_HEADER_SIZE;
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
    _create_valid_page(TEST_PAGE_READ_TWO_PAGES, PAGE_SIZE);
}

Test(TestPage, test_page_read, .init = create_valid_page) {
    TestId test_id = TEST_PAGE_READ;

    i32 fd = open(tmp_files[test_id], O_RDONLY | O_BINARY, 0666);

    Page page = page_read(0, fd);

    cr_assert(eq(i32, page.first_free_byte, PAGE_HEADER_SIZE));
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
    cr_assert(eq(i32, page1.first_free_byte, PAGE_HEADER_SIZE));
    cr_assert(eq(i32, page1.free_space, PAGE_PAYLOAD_SIZE));
    cr_assert(eq(i32, page1.page_id, 0));
    cr_assert_arr_eq(page1.payload, PAGE_ZERO_PAYLOAD, PAGE_PAYLOAD_SIZE);

    Page page2 = page_read(1, fd);
    cr_assert(eq(i32, page2.first_free_byte, PAGE_HEADER_SIZE));
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

        lseek(fd, HEADER_SIZE + PAGE_SIZE * i, SEEK_SET);
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
