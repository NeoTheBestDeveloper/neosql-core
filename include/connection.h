#pragma once

#include "driver/driver.h"

typedef struct {
    char* path;
    int32_t fd;
    Driver driver;
    bool is_active;
} Connection;

typedef enum {
    CONNECTION_OK = 0,
    CONNECTION_CANNOT_READ_FILE = 1,
    CONNECTION_CANNOT_WRITE_FILE = 2,
    CONNECTION_IVALID_OR_CORRUPTED_DATA = 3,
} ConnectionResultStatus;

typedef struct {
    Connection conn;
    ConnectionResultStatus status;
} ConnectionResult;

ConnectionResult connection_open(char const* path);
void connection_close(Connection*);
