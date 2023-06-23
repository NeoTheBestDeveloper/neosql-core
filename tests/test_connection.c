#include <unistd.h>

#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "../src/connection.h"

Test(TestConnection, test_connection_with_db_creating) {
    ConnectionResult res = connection_open("test_base.db");

    cr_assert(eq(i32, res.status, CONNECTION_OK));
    unlink("test_base.db");
}
