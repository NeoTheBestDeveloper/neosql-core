#include <unistd.h>

#include "criterion/assert.h"
#include "criterion/criterion.h"
#include "criterion/internal/assert.h"
#include "criterion/new/assert.h"

#include "database_driver/page.h"
#include "tests/utils.h"
#include "utils/os.h"

static const u16 page1_free_space_expected = 2837;
static const u16 page2_free_space_expected = 0;

Test(TestPage, test_page_read1)
{
    i32 payload_fd = open_db_file(PAGE1_PAYLOAD_BLOB_PATH);
    u8 page1_payload_expected[PAGE_PAYLOAD_SIZE];
    read(payload_fd, page1_payload_expected, PAGE_PAYLOAD_SIZE);
    close(payload_fd);

    i32 fd = open_db_file(TEST_DB_PATH);
    Page page = page_new(0, fd);

    cr_assert(eq(u16, page.free_space, page1_free_space_expected));
    cr_assert_arr_eq(page.payload, page1_payload_expected, PAGE_PAYLOAD_SIZE);

    page_free(&page);
    close(fd);
}

Test(TestPage, test_page_read2)
{
    i32 payload_fd = open_db_file(PAGE2_PAYLOAD_BLOB_PATH);
    u8 page2_payload_expected[PAGE_PAYLOAD_SIZE];
    read(payload_fd, page2_payload_expected, PAGE_PAYLOAD_SIZE);
    close(payload_fd);

    i32 fd = open_db_file(TEST_DB_PATH);
    Page page = page_new(1, fd);

    cr_assert(eq(u16, page.free_space, page2_free_space_expected));
    cr_assert_arr_eq(page.payload, page2_payload_expected, PAGE_PAYLOAD_SIZE);

    page_free(&page);
    close(fd);
}
