#pragma once

#include "driver/addr.h"

#define DATATYPE_SIZE(_d_)                                                    \
    (1llu + ((_d_.id == CHAR || _d_.id == VARCHAR) ? 2llu : 0llu))

typedef enum {
    CHAR = 0,
    VARCHAR = 1,
    TEXT = 2,
    MICROINT = 3,
    SMALLINT = 4,
    INT = 5,
    BIGINT = 6,
    BOOL = 7,
    REAL = 8,
    TIMESTAMP = 9,
} DataTypeId;

typedef struct {
    DataTypeId id;
    bool nullable;
    u16 max_size;
} DataType;

typedef struct {
    char* name;
    DataType type;
} Column;

typedef struct {
    char* name;
    Column* columns;
    u64 records_count;
    u8 columns_count;
    Addr first_record;
    Addr last_record;
} Table;

typedef struct {
    u8* bytes;
    u64 size;
} SerializedTable;

typedef enum {
    TABLE_OK = 0,
    TABLE_TOO_LONG_NAME = 1,
    TABLE_TOO_LONG_COLUMN_NAME = 2,
    TABLE_WITHOUT_COLUMNS_NOT_VALID = 3,
} TableResultStatus;

typedef struct {
    Table table;
    TableResultStatus status;
} TableResult;

TableResult table_new(const char* name, const Column* columns,
                      uint8_t columns_count);
void table_free(Table*);
void serialized_table_free(SerializedTable*);

Table table_deserialize(const u8* bytes, u64 size);
SerializedTable table_serialize(const Table*);
