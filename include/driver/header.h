#pragma once

#include <stdint.h>

#include "addr.h"

#define HEADER_SIZE (100)
#define NEOSQL_MAGIC ("NEOSQL")
#define HEADER_RESERVED_SIZE (77)

typedef enum {
    STORAGE_TYPE_list = 0,
    STORAGE_TYPE_btree = 1,
} StorageType;

typedef struct {
    Addr last_table;
    Addr first_table;
    int32_t pages_count;
    StorageType storage_type; // At file must be int8_t number.
} Header;

typedef enum {
    HEADER_ok = 0,
    HEADER_invalid_magic = 1,
    HEADER_invalid_storage_type = 2,
    HEADER_file_size_too_small = 3,
} HeaderResultStatus;

typedef struct {
    Header header;
    HeaderResultStatus status;
} HeaderResult;

// Return default header, which will be used for creating new database.
Header header_default(void);
Header header_new(StorageType storage_type);

HeaderResult header_read(int32_t fd);
HeaderResult header_write(Header, int32_t fd);
