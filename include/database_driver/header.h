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

#define DEFAULT_HEADER                                                        \
    ((Header) {                                                               \
        .pages_count = DEFAULT_PAGES_COUNT,                                   \
        .first_table = DEFAULT_FIRST_TABLE_ADDR,                              \
        .last_table = DEFAULT_LAST_TABLE_ADDR,                                \
        .cached_pages_count = DEFAULT_CACHED_PAGES_COUNT,                     \
    })
