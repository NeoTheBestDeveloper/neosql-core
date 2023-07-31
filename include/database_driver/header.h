#pragma once

#include "addr.h"

#define HEADER_SIZE (100)
#define HEADER_RESERVED (74)
#define HEADER_MAGIC ("NEOSQL")
#define HEADER_MAGIC_SIZE (6)

#define DEFAULT_CACHED_PAGES_COUNT (8192)
#define DEFAULT_PAGES_COUNT (0)
#define DEFAULT_FIRST_TABLE_ADDR (NULL_ADDR)
#define DEFAULT_LAST_TABLE_ADDR (NULL_ADDR)

typedef struct {
    u32 pages_count;
    Addr first_table;
    Addr last_table;
    u32 cached_pages_count;
} Header;

typedef enum {
    HEADER_OK = 0,
    HEADER_FILE_INVALID_SIZE = 1,
    HEADER_INVALID_MAGIC = 2,
} HeaderResultStatus;

typedef struct {
    HeaderResultStatus status;
    Header header;
} HeaderResult;

#define DEFAULT_HEADER                                                        \
    ((Header) {                                                               \
        .pages_count = DEFAULT_PAGES_COUNT,                                   \
        .first_table = DEFAULT_FIRST_TABLE_ADDR,                              \
        .last_table = DEFAULT_LAST_TABLE_ADDR,                                \
        .cached_pages_count = DEFAULT_CACHED_PAGES_COUNT,                     \
    })

// Read header from database file.
HeaderResult header_new(i32 fd);

void header_write(const Header*, i32 fd);
