#include <unistd.h>

#include "criterion/assert.h"
#include "criterion/criterion.h"
#include "criterion/internal/assert.h"
#include "criterion/new/assert.h"

#include "database_driver/database_defaults.h"
#include "database_driver/header.h"
#include "database_driver/page.h"
#include "nclib/stream.h"
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
    cr_assert(eq(i32, page.id, 0));

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
    cr_assert(eq(i32, page.id, 1));

    page_free(&page);
    close(fd);
}

Test(TestPage, test_page_write)
{
    WITH_TMP_FILE(tmp_fd)
    {
        i32 payload_fd = open_db_file(PAGE2_PAYLOAD_BLOB_PATH);
        u8 page2_payload_expected[PAGE_PAYLOAD_SIZE];
        read(payload_fd, page2_payload_expected, PAGE_PAYLOAD_SIZE);
        close(payload_fd);

        u8 header_fake[HEADER_SIZE] = { 0 };
        write(tmp_fd, header_fake, HEADER_SIZE);

        Page page = {
            .free_space = 0,
            .payload = page2_payload_expected,
            .id = 0,
        };

        page_write(&page, tmp_fd);

        lseek(tmp_fd, 100, SEEK_SET);

        u8 page_written_buf[PAGE_SIZE];
        read(tmp_fd, page_written_buf, PAGE_SIZE);

        Stream stream = stream_new(page_written_buf, PAGE_HEADER_SIZE,
                                   DEFAULT_DATABASE_ENDIAN);
        u16 free_space_written = stream_read_u16(&stream);

        cr_assert(eq(u16, page.free_space, free_space_written));
        cr_assert_arr_eq(page_written_buf + PAGE_HEADER_SIZE,
                         page2_payload_expected, PAGE_PAYLOAD_SIZE);
    }
}
