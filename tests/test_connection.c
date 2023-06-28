#include <stdio.h>
#include <unistd.h>

#include "criterion/criterion.h"
#include "criterion/internal/new_asserts.h"
#include "criterion/new/assert.h"

#include "connection.h"

Test(TestConnection, test_connection_open_db) {
    ConnectionResult res = connection_open("../tests/assets/test_database.db");

    cr_assert(eq(u32, res.status, CONNECTION_OK));

    Connection conn = res.conn;

    cr_assert(conn.is_active);

    Driver driver = conn.driver;

    cr_assert(eq(i32, driver.header.pages_count, 1));
    cr_assert(eq(u32, driver.header.storage_type, 0));
    cr_assert(addr_cmp(driver.header.first_table, NULL_ADDR));
    cr_assert(addr_cmp(driver.header.last_table, NULL_ADDR));

    connection_close(&(res.conn));
}

Test(TestConnection, test_connection_open_empty_db) {
    ConnectionResult res = connection_open("tmp_file.db");

    cr_assert(eq(u32, res.status, CONNECTION_OK));

    Connection conn = res.conn;

    cr_assert(conn.is_active);

    connection_close(&res.conn);
    unlink("tmp_file.db");
}
