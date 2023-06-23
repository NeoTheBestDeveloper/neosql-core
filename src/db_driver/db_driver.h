#pragma once

#include <types.h>

#include "page.h"

#include "db_header.h"

// #define MAX_CACHED_PAGES_CNT (1024)

// typedef struct {
//     Page *page;
//     bool is_changed;
// } CachedPage;

// CachedPage *cached_pages; // TODO: cache container instead this.
// u16 cached_pages_cnt;

typedef struct {
    i32 db_fd;
    DbHeader header;
} DbDriver;

typedef enum {
    DB_DRIVER_OK = 0,
    DB_DRIVER_INVALID_OR_CORRUPTED_HEADER = 1,
} DbDriverResultStatus;

typedef struct {
    DbDriver driver;
    DbDriverResultStatus status;
} DbDriverResult;

DbDriverResult db_driver_create_db(i32 db_fd);
DbDriverResult db_driver_open_db(i32 db_fd);

void db_driver_free(DbDriver *driver);
