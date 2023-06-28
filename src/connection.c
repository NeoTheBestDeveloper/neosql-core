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

static ConnectionResult _open_existen_db(char const *path) {
    ConnectionResult res = {
        .status = CONNECTION_OK,
    };

    if (!is_file_writable(path)) {
        res.status = CONNECTION_CANNOT_WRITE_FILE;
        return res;
    }

    if (!is_file_readable(path)) {
        res.status = CONNECTION_CANNOT_READ_FILE;
        return res;
    }

    int32_t fd = open(path, O_RDWR | O_BINARY, 0700);
    DriverResult driver_res = driver_open_db(fd);

    if (driver_res.status == DRIVER_INVALID_OR_CORRUPTED_HEADER) {
        res.status = CONNECTION_IVALID_OR_CORRUPTED_DATA;
        return res;
    }

    res.conn.driver = driver_res.driver;

    return res;
}

static ConnectionResult _open_noexisten_db(char const *path) {
    ConnectionResult res = {
        .status = CONNECTION_OK,
    };

    int32_t fd = open(path, O_RDWR | O_CREAT | O_BINARY, 0700);

    if (fd < 0 && errno == EACCES) {
        res.status = CONNECTION_CANNOT_WRITE_FILE;
        return res;
    }

    if (!is_file_readable(path)) {
        res.status = CONNECTION_CANNOT_READ_FILE;
        return res;
    }

    res.conn.driver = driver_create_db(fd);

    return res;
}

ConnectionResult connection_open(char const *path) {
    ConnectionResult res = (is_file_exists(path)) ? _open_existen_db(path)
                                                  : _open_noexisten_db(path);

    if (res.status == CONNECTION_OK) {
        res.conn.fd = res.conn.driver.fd;
        res.conn.is_active = true;
        res.conn.path = strdup(path);
    }

    return res;
}

void connection_close(Connection *conn) {
    conn->is_active = false;

    close(conn->fd);
    free(conn->path);

    driver_free(&(conn->driver));
}
