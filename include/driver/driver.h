#pragma once

#include "header.h"
#include "page.h"
#include "table.h"

// TODO: Realize cache for pages.
// TODO: use arena allocator for cached pages.

typedef struct {
    Header header;
    i32 fd;
} Driver;

typedef enum {
    DRIVER_OK = 0,
    DRIVER_INVALID_OR_CORRUPTED_HEADER = 1,
} DriverResultStatus;

typedef struct {
    Driver driver;
    DriverResultStatus status;
} DriverResult;

typedef enum {
    TABLE_SEARCH_OK = 0,
    TABLE_SEARCH_NOT_FOUND = 1,
} TableSearchResultStatus;

typedef struct {
    Table table;
    TableSearchResultStatus status;
} TableSearchResult;

/* Create new database inside file with default header and write
 "DEFAULT_EMPTY_DB_ZERO_PAGES_COUNT" to the file. */
Driver driver_create_db(i32 fd);

// Trying to read header from already existing database.
DriverResult driver_open_db(i32 fd);

void driver_free(Driver*);

void driver_append_table(Driver*, const Table*);
TableSearchResult driver_find_table(const Driver*, const char*);

// driver_delete_table();
// driver_update_table();
