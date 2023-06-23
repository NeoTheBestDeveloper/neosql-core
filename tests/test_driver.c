#include <fcntl.h>
#include <unistd.h>

#include <os.h>

#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "../src/db_driver/db_driver.h"

typedef enum {
    TEST_DRIVER_DB_CREATING = 0,
    TEST_DRIVER_DB_OPENING = 1,
    TEST_DRIVER_DB_OPENING_IVALID_DATA = 2,
} TestId;

char tmp_files[3][100] = {
    "test_tmp_file_test_driver_0.db",
    "test_tmp_file_test_driver_1.db",
    "test_tmp_file_test_driver_2.db",
};

static void delete_tmp_file(TestId test_id) { unlink(tmp_files[test_id]); }

// DbDriverResult db_driver_create_db(i32 db_fd);
// DbDriverResult db_driver_open_db(i32 db_fd);

Test(TestDriver, test_driver_db_creating) {
    TestId test_id = TEST_DRIVER_DB_CREATING;
    i32 fd = open(tmp_files[test_id], O_CREAT | O_BINARY | O_RDWR, 0666);

    DbDriverResult res = db_driver_create_db(fd);

    cr_assert(eq(u32, res.status, DB_DRIVER_OK));
    lseek(fd, 0, SEEK_SET);

    db_driver_free(&res.driver);
    delete_tmp_file(test_id);
    close(fd);
}
