#pragma once

#include "header.h"
#include "page.h"

typedef struct {
    Header header;
    int32_t fd;
} Driver;

typedef enum {
    DRIVER_ok = 0,
    DRIVER_invalid_or_corrupted_header = 1,
} DriverResultStatus;

typedef struct {
    Driver driver;
    DriverResultStatus status;
} DriverResult;

/* Create new database inside file with default header and write
 "DEFAULT_EMPTY_DB_ZERO_PAGES_COUNT" to the file. */
DriverResult driver_create_db(int32_t fd);

// Trying to read header from already existing database.
DriverResult driver_open_db(int32_t fd);

void driver_free(Driver *);
