#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "criterion/assert.h"
#include "criterion/criterion.h"
#include "criterion/internal/assert.h"
#include "criterion/internal/test.h"
#include "criterion/new/assert.h"

#include "../../src/db_driver/page.h"

typedef enum {
  TEST_PAGE_READ_4KB = 0,
  TEST_PAGE_READ_8KB = 1,
  TEST_PAGE_READ_16KB = 2,
  TEST_PAGE_WRITE_4KB = 3,
  TEST_PAGE_WRITE_8KB = 4,
  TEST_PAGE_WRITE_16KB = 5,
  TEST_PAGE_READ_TWO_PAGES = 6,
  TEST_PAGE_WRITE_TWO_PAGES = 7,
} TestId;

char tmp_files[8][100 + 1] = {
    "tmp_file_test_page_id_0.db", "tmp_file_test_page_id_1.db",
    "tmp_file_test_page_id_2.db", "tmp_file_test_page_id_3.db",
    "tmp_file_test_page_id_4.db", "tmp_file_test_page_id_5.db",
    "tmp_file_test_page_id_6.db", "tmp_file_test_page_id_7.db",
};

u8 header_mock[100] = {0};
u8 FOUR_KB_PAGE_MOCK[4 * 1024 - PAGE_HEADER_SIZE] = {0};
u8 EIGT_KB_PAGE_MOCK[8 * 1024 - PAGE_HEADER_SIZE] = {0};
u8 SIXTEEN_KB_PAGE_MOCK[16 * 1024 - PAGE_HEADER_SIZE] = {0};

void delete_tmp_file(TestId test_id) { unlink(tmp_files[test_id]); }

void _create_valid_page(TestId test_id, PageSize page_size, u64 page_offset) {
  i32 fd = open(tmp_files[test_id], O_CREAT | O_RDWR, 0666);

  u16 free_space = page_size_to_bytes(page_size) - PAGE_HEADER_SIZE;
  u16 first_free_byte = PAGE_HEADER_SIZE;
  u8 reserved[PAGE_HEADER_SIZE - 4] = {0};

  write(fd, header_mock, 100);
  lseek(fd, page_offset, SEEK_CUR);

  write(fd, &free_space, 2);
  write(fd, &first_free_byte, 2);
  write(fd, reserved, PAGE_HEADER_SIZE - 4);

  if (page_size == FOUR_KB) {
    write(fd, FOUR_KB_PAGE_MOCK, 4 * 1024 - PAGE_HEADER_SIZE);
  } else if (page_size == EIGHT_KB) {
    write(fd, EIGT_KB_PAGE_MOCK, 8 * 1024 - PAGE_HEADER_SIZE);
  } else {
    write(fd, SIXTEEN_KB_PAGE_MOCK, 16 * 1024 - PAGE_HEADER_SIZE);
  }

  close(fd);
}

void create_valid_4KB_page(void) {
  _create_valid_page(TEST_PAGE_READ_4KB, FOUR_KB, 0);
}

void create_valid_8KB_page(void) {
  _create_valid_page(TEST_PAGE_READ_8KB, EIGHT_KB, 0);
}

void create_valid_16KB_page(void) {
  _create_valid_page(TEST_PAGE_READ_16KB, SIXTEEN_KB, 0);
}

void create_valid_two_4KB_pages(void) {
  _create_valid_page(TEST_PAGE_READ_TWO_PAGES, FOUR_KB, 0);
  _create_valid_page(TEST_PAGE_READ_TWO_PAGES, FOUR_KB,
                     page_size_to_bytes(FOUR_KB));
}

Test(TestPage, test_page_read_4KB, .init = create_valid_4KB_page) {
  i32 test_id = TEST_PAGE_READ_4KB;

  i32 fd = open(tmp_files[test_id], R_OK, 0666);

  Page page = page_read(FOUR_KB, 0, fd);

  cr_assert(eq(i32, page.page_size, FOUR_KB));
  cr_assert(eq(i32, page.first_free_byte, PAGE_HEADER_SIZE));
  cr_assert(
      eq(i32, page.free_space, page_size_to_bytes(FOUR_KB) - PAGE_HEADER_SIZE));
  cr_assert(eq(i32, page.page_id, 0));

  cr_assert(eq(u64, page.writer.buf_size,
               page_size_to_bytes(FOUR_KB) - PAGE_HEADER_SIZE));
  cr_assert_arr_eq(page_get_buf(&page), FOUR_KB_PAGE_MOCK,
                   page_size_to_bytes(FOUR_KB) - PAGE_HEADER_SIZE);

  close(fd);
  delete_tmp_file(test_id);
}

Test(TestPage, test_page_read_8KB, .init = create_valid_8KB_page) {
  i32 test_id = TEST_PAGE_READ_8KB;

  i32 fd = open(tmp_files[test_id], R_OK, 0666);

  Page page = page_read(EIGHT_KB, 0, fd);

  cr_assert(eq(i32, page.page_size, EIGHT_KB));
  cr_assert(eq(i32, page.first_free_byte, PAGE_HEADER_SIZE));
  cr_assert(eq(i32, page.free_space,
               page_size_to_bytes(EIGHT_KB) - PAGE_HEADER_SIZE));
  cr_assert(eq(i32, page.page_id, 0));

  cr_assert(eq(u64, page.writer.buf_size,
               page_size_to_bytes(EIGHT_KB) - PAGE_HEADER_SIZE));
  cr_assert_arr_eq(page_get_buf(&page), EIGT_KB_PAGE_MOCK,
                   page_size_to_bytes(EIGHT_KB) - PAGE_HEADER_SIZE);

  close(fd);
  delete_tmp_file(test_id);
}

Test(TestPage, test_page_read_16KB, .init = create_valid_16KB_page) {
  i32 test_id = TEST_PAGE_READ_16KB;

  i32 fd = open(tmp_files[test_id], R_OK, 0666);

  Page page = page_read(SIXTEEN_KB, 0, fd);

  cr_assert(eq(i32, page.page_size, SIXTEEN_KB));
  cr_assert(eq(i32, page.first_free_byte, PAGE_HEADER_SIZE));
  cr_assert(eq(i32, page.free_space,
               page_size_to_bytes(SIXTEEN_KB) - PAGE_HEADER_SIZE));
  cr_assert(eq(i32, page.page_id, 0));

  cr_assert(eq(u64, page.writer.buf_size,
               page_size_to_bytes(SIXTEEN_KB) - PAGE_HEADER_SIZE));
  cr_assert_arr_eq(page_get_buf(&page), SIXTEEN_KB_PAGE_MOCK,
                   page_size_to_bytes(SIXTEEN_KB) - PAGE_HEADER_SIZE);

  close(fd);
  delete_tmp_file(test_id);
}

Test(TestPage, test_page_read_two_pages, .init = create_valid_two_4KB_pages) {
  i32 test_id = TEST_PAGE_READ_TWO_PAGES;
  i32 fd = open(tmp_files[test_id], R_OK, 0666);

  Page page1 = page_read(FOUR_KB, 0, fd);

  cr_assert(eq(i32, page1.page_size, FOUR_KB));
  cr_assert(eq(i32, page1.first_free_byte, PAGE_HEADER_SIZE));
  cr_assert(eq(i32, page1.free_space,
               page_size_to_bytes(FOUR_KB) - PAGE_HEADER_SIZE));
  cr_assert(eq(i32, page1.page_id, 0));

  cr_assert(eq(u64, page1.writer.buf_size,
               page_size_to_bytes(FOUR_KB) - PAGE_HEADER_SIZE));
  cr_assert_arr_eq(page_get_buf(&page1), FOUR_KB_PAGE_MOCK,
                   page_size_to_bytes(FOUR_KB) - PAGE_HEADER_SIZE);

  Page page2 = page_read(FOUR_KB, 1, fd);

  cr_assert(eq(i32, page2.page_size, FOUR_KB));
  cr_assert(eq(i32, page2.first_free_byte, PAGE_HEADER_SIZE));
  cr_assert(eq(i32, page2.free_space,
               page_size_to_bytes(FOUR_KB) - PAGE_HEADER_SIZE));
  cr_assert(eq(i32, page2.page_id, 1));

  cr_assert(eq(u64, page2.writer.buf_size,
               page_size_to_bytes(FOUR_KB) - PAGE_HEADER_SIZE));
  cr_assert_arr_eq(page_get_buf(&page2), FOUR_KB_PAGE_MOCK,
                   page_size_to_bytes(FOUR_KB) - PAGE_HEADER_SIZE);

  close(fd);
  delete_tmp_file(test_id);
}

Test(TestPage, test_page_write_4KB) {
  i32 test_id = TEST_PAGE_WRITE_4KB;

  i32 fd = open(tmp_files[test_id], O_RDWR | O_CREAT, 0666);
  write(fd, header_mock, HEADER_SIZE);
  close(fd);

  fd = open(tmp_files[test_id], O_RDWR, 0666);

  Page page = page_new(FOUR_KB, 0);

  page_write(&page, fd);

  u8 page_buf[page_size_to_bytes(FOUR_KB) - PAGE_HEADER_SIZE];
  u16 free_space_buf;
  u16 first_free_byte_buf;

  lseek(fd, HEADER_SIZE, SEEK_SET);
  read(fd, &free_space_buf, 2);
  read(fd, &first_free_byte_buf, 2);
  read(fd, page_buf, page_size_to_bytes(FOUR_KB) - PAGE_HEADER_SIZE);

  cr_assert(eq(u16, page.free_space, free_space_buf));
  cr_assert(eq(u16, page.first_free_byte, first_free_byte_buf));
  cr_assert_arr_eq(page_get_buf(&page), page_buf,
                   page_size_to_bytes(FOUR_KB) - PAGE_HEADER_SIZE);

  close(fd);
  delete_tmp_file(test_id);
}

Test(TestPage, test_page_write_8KB) {
  i32 test_id = TEST_PAGE_WRITE_8KB;

  i32 fd = open(tmp_files[test_id], O_RDWR | O_CREAT, 0666);
  write(fd, header_mock, HEADER_SIZE);
  close(fd);

  fd = open(tmp_files[test_id], O_RDWR, 0666);

  Page page = page_new(EIGHT_KB, 0);

  page_write(&page, fd);

  u8 page_buf[page_size_to_bytes(EIGHT_KB) - PAGE_HEADER_SIZE];
  u16 free_space_buf;
  u16 first_free_byte_buf;

  lseek(fd, HEADER_SIZE, SEEK_SET);
  read(fd, &free_space_buf, 2);
  read(fd, &first_free_byte_buf, 2);
  read(fd, page_buf, page_size_to_bytes(EIGHT_KB) - PAGE_HEADER_SIZE);

  cr_assert(eq(u16, page.free_space, free_space_buf));
  cr_assert(eq(u16, page.first_free_byte, first_free_byte_buf));
  cr_assert_arr_eq(page_get_buf(&page), page_buf,
                   page_size_to_bytes(EIGHT_KB) - PAGE_HEADER_SIZE);

  close(fd);
  delete_tmp_file(test_id);
}

Test(TestPage, test_page_write_16KB) {
  i32 test_id = TEST_PAGE_WRITE_16KB;

  i32 fd = open(tmp_files[test_id], O_RDWR | O_CREAT, 0666);
  write(fd, header_mock, HEADER_SIZE);
  close(fd);

  fd = open(tmp_files[test_id], O_RDWR, 0666);

  Page page = page_new(SIXTEEN_KB, 0);

  page_write(&page, fd);

  u8 page_buf[page_size_to_bytes(SIXTEEN_KB) - PAGE_HEADER_SIZE];
  u16 free_space_buf;
  u16 first_free_byte_buf;

  lseek(fd, HEADER_SIZE, SEEK_SET);
  read(fd, &free_space_buf, 2);
  read(fd, &first_free_byte_buf, 2);
  read(fd, page_buf, page_size_to_bytes(SIXTEEN_KB) - PAGE_HEADER_SIZE);

  cr_assert(eq(u16, page.free_space, free_space_buf));
  cr_assert(eq(u16, page.first_free_byte, first_free_byte_buf));
  cr_assert_arr_eq(page_get_buf(&page), page_buf,
                   page_size_to_bytes(SIXTEEN_KB) - PAGE_HEADER_SIZE);

  close(fd);
  delete_tmp_file(test_id);
}

Test(TestPage, test_page_write_two_pages) {
  i32 test_id = TEST_PAGE_WRITE_TWO_PAGES;

  i32 fd = open(tmp_files[test_id], O_RDWR | O_CREAT, 0666);
  write(fd, header_mock, HEADER_SIZE);
  close(fd);

  fd = open(tmp_files[test_id], O_RDWR, 0666);

  Page page1 = page_new(SIXTEEN_KB, 0);
  Page page2 = page_new(SIXTEEN_KB, 1);

  page_write(&page1, fd);
  page_write(&page2, fd);

  for (i32 i = 0; i < 2; ++i) {
    Page page = (i == 0) ? page1 : page2;

    u8 page_buf[page_size_to_bytes(SIXTEEN_KB) - PAGE_HEADER_SIZE];
    u16 free_space_buf;
    u16 first_free_byte_buf;

    lseek(fd, HEADER_SIZE + page_size_to_bytes(SIXTEEN_KB) * i, SEEK_SET);
    read(fd, &free_space_buf, 2);
    read(fd, &first_free_byte_buf, 2);
    read(fd, page_buf, page_size_to_bytes(SIXTEEN_KB) - PAGE_HEADER_SIZE);

    cr_assert(eq(u16, page.free_space, free_space_buf));
    cr_assert(eq(u16, page.first_free_byte, first_free_byte_buf));
    cr_assert_arr_eq(page_get_buf(&page), page_buf,
                     page_size_to_bytes(SIXTEEN_KB) - PAGE_HEADER_SIZE);
  }

  close(fd);
  delete_tmp_file(test_id);
}
