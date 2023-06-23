#pragma once

#include <types.h>

#include "addr.h"

#define NEOSQL_MAGIC ("NEOSQL")
#define HEADER_SIZE (100)
#define HEADER_RESERVED_SIZE (77)

typedef enum {
    LIST_BLOCKS = 0,
    B_TREE_BLOCKS = 1,
} StorageType;

typedef struct {
    u32 pages_count;
    StorageType storage_type; // At file must be u8 number.
    Addr first_table;
    Addr last_table;
} DbHeader;

typedef enum {
    DB_HEADER_OK = 0,
    DB_HEADER_INVALID_MAGIC = 1,
    DB_HEADER_INVALID_STORAGE_TYPE = 2,
    DB_HEADER_HEADER_SIZE_TOO_SMALL = 3,
} DbHeaderResultStatus;

typedef struct {
    DbHeader header;
    DbHeaderResultStatus status;
} DbHeaderResult;

DbHeader db_header_new(u32 pages_count, StorageType storage_type,
                       Addr first_table, Addr last_table);
DbHeader db_header_new_default(void);

DbHeaderResult db_header_read(i32 fd);
DbHeaderResult db_header_write(const DbHeader *header, i32 fd);
