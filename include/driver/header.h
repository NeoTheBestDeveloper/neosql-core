#pragma once

#include "addr.h"

#define HEADER_SIZE (100)
#define NEOSQL_MAGIC ("NEOSQL")
#define HEADER_RESERVED_SIZE (77)

typedef enum {
    STORAGE_TYPE_LIST = 0,
    STORAGE_TYPE_BTREE = 1,
} StorageType;

typedef struct {
    Addr first_table;
    Addr last_table;
    i32 pages_count;
    StorageType storage_type; // At file must be int8_t number.
} Header;

typedef enum {
    HEADER_OK = 0,
    HEADER_INVALID_MAGIC = 1,
    HEADER_INVALID_STORAGE_TYPE = 2,
    HEADER_FILE_SIZE_TOO_SMALL = 3,
} HeaderResultStatus;

typedef struct {
    Header header;
    HeaderResultStatus status;
} HeaderResult;

// Return default header, which will be used for creating new database.
Header header_default(void);
Header header_new(StorageType storage_type);

HeaderResult header_read(i32 fd);
void header_write(Header const*, i32 fd);
