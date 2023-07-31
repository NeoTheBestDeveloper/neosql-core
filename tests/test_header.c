#include <unistd.h>

#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "nclib.h"

#include "database_driver/header.h"
#include "tests/utils.h"
#include "utils/os.h"

u8* header_serialise(const Header*);
Header header_deserialise(u8*);

static u8 expected_serialised_header[100] = {
    0x4e, 0x45, 0x4f, 0x53, 0x51, 0x4c, 0x0,  0x0,  0x0,  0x0,  0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0,  0x20,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,
};
static Header expected_header = DEFAULT_HEADER;

Test(TestHeader, test_header_serialisation)
{
    u8* serialised_header = header_serialise(&expected_header);
    cr_assert_arr_eq(serialised_header, expected_serialised_header,
                     HEADER_SIZE);

    free(serialised_header);
}

Test(TestHeader, test_header_deserialisation)
{
    const Header header = header_deserialise(expected_serialised_header);

    cr_assert(eq(u32, header.cached_pages_count,
                 expected_header.cached_pages_count));
    cr_assert(eq(u32, header.pages_count, expected_header.pages_count));
    cr_assert(addr_eq(header.first_table, expected_header.first_table));
    cr_assert(addr_eq(header.last_table, expected_header.last_table));
}

Test(TestHeader, test_header_successfull_read)
{
    i32 fd = open_db_file(TEST_DB_PATH);

    HeaderResult res = header_new(fd);

    cr_assert(eq(u32, res.status, HEADER_OK));

    Header header = res.header;
    cr_assert(eq(u32, header.cached_pages_count, DEFAULT_CACHED_PAGES_COUNT));
    cr_assert(eq(u32, header.pages_count, 2));
    cr_assert(addr_eq(header.first_table, (Addr) { 0, 0 }));
    cr_assert(
        addr_eq(header.last_table, (Addr) { .page_id = 0, .offset = 70 }));

    close(fd);
}

Test(TestHeader, test_header_successfull_write)
{
    WITH_TMP_FILE(tmp_fd)
    {
        Header header = DEFAULT_HEADER;
        header_write(&header, tmp_fd);
    }
}
