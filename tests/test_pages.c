#include <stdlib.h>
#include <unistd.h>

#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "database_driver/header.h"
#include "nclib.h"

#include "database_driver/pages.h"
#include "tests/utils.h"
#include "utils/os.h"

static const u16 page1_free_space_expected = 2837;
static const u16 page2_free_space_expected = 0;

static u8* get_page1_payload()
{
    i32 payload_fd = open_db_file(PAGE1_PAYLOAD_BLOB_PATH);
    u8* page1_payload = malloc(PAGE_PAYLOAD_SIZE);
    read(payload_fd, page1_payload, PAGE_PAYLOAD_SIZE);
    close(payload_fd);

    return page1_payload;
}

static u8* get_page2_payload()
{
    i32 payload_fd = open_db_file(PAGE2_PAYLOAD_BLOB_PATH);
    u8* page2_payload = malloc(PAGE_PAYLOAD_SIZE);
    read(payload_fd, page2_payload, PAGE_PAYLOAD_SIZE);
    close(payload_fd);

    return page2_payload;
}

Test(TestPages, test_get_pages_free_space)
{
    i32 fd = open_db_file(TEST_DB_PATH);

    Pages pages = pages_new(fd, 2, DEFAULT_CACHED_PAGES_COUNT);

    u16 page1_free_space = pages_get_page_free_space(&pages, 0);
    u16 page2_free_space = pages_get_page_free_space(&pages, 1);

    cr_assert(eq(u16, page1_free_space, page1_free_space_expected));
    cr_assert(eq(u16, page2_free_space, page2_free_space_expected));

    pages_free(&pages);
    close(fd);
}

Test(TestPages, test_get_page1_payload)
{
    u8* payload_expected = get_page1_payload();

    free(payload_expected);
}
