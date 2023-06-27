#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "connection.h"
#include "driver/driver.h"
#include "os.h"

static bool is_file_exists(const char *path) { return 0 == access(path, F_OK); }
static bool is_file_writable(const char *path) {
    return 0 == access(path, W_OK);
}
static bool is_file_readable(const char *path) {
    return 0 == access(path, R_OK);
}

ConnectionResult connection_open(const char *path) {
    Connection conn;
    DriverResult driver_res;

    ConnectionResult res = {
        .status = CONNECTION_ok,
    };

    int32_t fd;
    if (is_file_exists(path)) {
        if (!is_file_writable(path)) {
            res.status = CONNECTION_cannot_write_file;
            return res;
        }

        if (!is_file_readable(path)) {
            res.status = CONNECTION_cannot_read_file;
            return res;
        }

        fd = open(path, O_RDWR | O_BINARY);
        driver_res = driver_open_db(fd);
        if (driver_res.status == DRIVER_invalid_or_corrupted_header) {
            res.status = CONNECTION_ivalid_or_corrupted_file;
            return res;
        }

        conn.driver = driver_res.driver;
        conn.has_db = true;
    } else {
        fd = open(path, O_RDWR | O_CREAT | O_BINARY, 0700);

        if (fd < 0 && errno == EACCES) {
            res.status = CONNECTION_cannot_write_file;
            return res;
        }

        if (!is_file_readable(path)) {
            res.status = CONNECTION_cannot_read_file;
            return res;
        }
        conn.has_db = false;
    }

    conn.fd = fd;
    conn.path = strdup(path);
    conn.is_active = true;

    res.conn = conn;
    return res;
}

void connection_close(Connection *conn) {
    conn->is_active = false;

    close(conn->fd);
    free(conn->path);

    driver_free(&(conn->driver));
}
