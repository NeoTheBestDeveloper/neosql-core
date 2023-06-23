#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <os.h>

#include "connection.h"
#include "src/db_driver/db_driver.h"

static bool is_file_exists(const char *path) { return 0 == access(path, F_OK); }
static bool is_file_writable(const char *path) {
    return 0 == access(path, W_OK);
}
static bool is_file_readable(const char *path) {
    return 0 == access(path, R_OK);
}

ConnectionResult connection_open(const char *path) {
    Connection conn;
    DbDriverResult driver_res;
    ConnectionResult res = {
        .status = CONNECTION_OK,
    };

    i32 fd;
    if (is_file_exists(path)) {
        if (!is_file_writable(path)) {
            res.status = CONNECTION_ERROR_CANNOT_WRITE_FILE;
            return res;
        }

        if (!is_file_readable(path)) {
            res.status = CONNECTION_ERROR_CANNOT_READ_FILE;
            return res;
        }

        fd = open(path, O_RDWR | O_BINARY);
        driver_res = db_driver_open_db(fd);
        if (driver_res.status == DB_DRIVER_INVALID_OR_CORRUPTED_HEADER) {
            res.status = CONNECTION_ERROR_IVALID_OR_CORRUPTED_FILE;
            return res;
        }

    } else {
        fd = open(path, O_RDWR | O_CREAT | O_BINARY, 0700);

        if (fd < 0 && errno == EACCES) {
            res.status = CONNECTION_ERROR_CANNOT_WRITE_FILE;
            return res;
        }

        if (!is_file_readable(path)) {
            res.status = CONNECTION_ERROR_CANNOT_READ_FILE;
            return res;
        }

        driver_res = db_driver_create_db(fd);
    }

    conn.fd = fd;
    conn.db_driver = driver_res.driver;
    conn.path = strdup(path);
    conn.is_active = true;

    res.conn = conn;
    return res;
}

void connection_close(Connection *conn) {
    conn->is_active = false;

    close(conn->fd);
    free(conn->path);
    db_driver_free(&conn->db_driver);
}
