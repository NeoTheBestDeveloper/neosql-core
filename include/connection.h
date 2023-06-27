#pragma once

#include "driver/driver.h"

typedef struct {
    char *path;
    int32_t fd;
    bool has_db;
    bool is_active;
    Driver driver;
} Connection;

typedef enum {
    CONNECTION_ok = 0,
    CONNECTION_cannot_read_file = 1,
    CONNECTION_cannot_write_file = 2,
    CONNECTION_ivalid_or_corrupted_file = 3,
} ConnectionResultStatus;

typedef struct {
    Connection conn;
    ConnectionResultStatus status;
} ConnectionResult;

ConnectionResult connection_open(char const *path);
void connection_close(Connection *conn);
