#pragma once

#include <types.h>

#include "addr.h"

#define NEOSQL_MAGIC ("NEOSQL")
#define HEADER_SIZE (100)

typedef enum {
    LIST = 0,
    B_TREE = 1,
} StorageType;

typedef enum {
    FOUR_KB = 0,
    EIGHT_KB = 1,
    SIXTEEN_KB = 2,
} PageSize;

typedef struct {
    u32 pages_count;
    StorageType storage_type; // At file must be u8 number.
    PageSize page_size;       // At file must be u8 number.
    Addr first_table_node;
} DbHeader;

typedef enum {
    DB_HEADER_OK = 0,
    DB_HEADER_INVALID_MAGIC = 1,
    DB_HEADER_INVALID_STORAGE_TYPE = 2,
    DB_HEADER_INVALID_PAGE_SIZE = 3,
    DB_HEADER_HEADER_SIZE_TOO_SMALL = 4,
} DbHeaderResultStatus;

typedef struct {
    DbHeader header;
    DbHeaderResultStatus status;
} DbHeaderResult;

DbHeader db_header_new(u32 pages_count, StorageType storage_type,
                       PageSize page_size, Addr first_table_node);
DbHeader db_header_new_default(void);
DbHeaderResult db_header_read(i32 fd);
DbHeaderResult db_header_write(const DbHeader *header, i32 fd);

u64 page_size_to_bytes(PageSize page_size);
