#pragma once

#include <types.h>

#include "db_driver/db_driver.h"

typedef struct {
    char *path;
    DbDriver db_driver;
    i32 fd;
    bool is_active;
    bool has_db;
} Connection;

typedef enum {
    CONNECTION_OK = 0,
    CONNECTION_ERROR_CANNOT_READ_FILE = 1,
    CONNECTION_ERROR_CANNOT_WRITE_FILE = 2,
    CONNECTION_ERROR_IVALID_OR_CORRUPTED_FILE = 3,
} ConnectionResultStatus;

typedef struct {
    Connection conn;
    ConnectionResultStatus status;
} ConnectionResult;

ConnectionResult connection_open(const char *path);
void connection_close(Connection *conn);
