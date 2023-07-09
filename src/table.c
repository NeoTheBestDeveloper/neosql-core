#include <stdlib.h>
#include <string.h>

#include "database_constrains.h"
#include "driver/list_block.h"
#include "table.h"
#include "utils/buf_reader.h"
#include "utils/buf_writer.h"
#include "utils/mem.h"

// Private methods signatures start.
static TableResultStatus
table_validate(const char* name, const Column* columns, u8 columns_count);

static void table_serialize_name(const char* name, BufWriter* writer);
static void table_serialize_columns(const Column* columns, u8 columns_count,
                                    BufWriter* writer);

static void table_deserialize_name(char** name, BufReader* reader);
static void table_deserialize_columns(Column** columns, u8 columns_count,
                                      BufReader* reader);
// Private methods signatures end.

// Public methods start.
TableResult table_new(const char* name, const Column* columns,
                      u8 columns_count)
{

    TableResultStatus validation_status
        = table_validate(name, columns, columns_count);

    if (validation_status != TABLE_OK) {
        return (TableResult) { .status = validation_status };
    }

    Table t = {
        .name = strdup(name),
        .columns_count = columns_count,
        .columns = copy_mem(columns, columns_count * sizeof(Column)),
        .records_count = 0,
        .first_record = NULL_ADDR,
        .last_record = NULL_ADDR,
    };

    for (u8 i = 0; i < columns_count; ++i) {
        t.columns[i].name = strdup(columns[i].name);
    }

    return (TableResult) { .table = t, .status = TABLE_OK };
}

void table_free(Table* t)
{
    for (u8 i = 0; i < t->columns_count; ++i) {
        free(t->columns[i].name);
    }
    free(t->columns);
    free(t->name);
}

SerializedTable table_serialize(const Table* t)
{
    u64 size = table_serialized_size(t);
    u8* bytes = malloc(size);

    BufWriter writer = buf_writer_new(bytes, size);

    buf_writer_write(&writer, &(t->first_record), 6);
    buf_writer_write(&writer, &(t->last_record), 6);

    buf_writer_write(&writer, &(t->columns_count), 1);
    buf_writer_write(&writer, &(t->records_count), 8);

    table_serialize_name(t->name, &writer);
    table_serialize_columns(t->columns, t->columns_count, &writer);

    return (SerializedTable) { .bytes = bytes, .size = size };
}

Table table_deserialize(const u8* bytes, u64 size)
{
    BufReader reader = buf_reader_new(bytes, size);

    Table table;
    buf_reader_read(&reader, &(table.first_record), 6);
    buf_reader_read(&reader, &(table.last_record), 6);

    buf_reader_read(&reader, &(table.columns_count), 1);
    buf_reader_read(&reader, &(table.records_count), 8);

    table_deserialize_name(&(table.name), &reader);
    table_deserialize_columns(&(table.columns), table.columns_count, &reader);

    return table;
}

void serialized_table_free(SerializedTable* t) { free(t->bytes); }
// Public methods end.

// Private methods start.
static TableResultStatus
table_validate(const char* name, const Column* columns, u8 columns_count)
{
    if (columns_count == 0) {
        return TABLE_WITHOUT_COLUMNS_NOT_VALID;
    }

    u64 name_len = strlen(name);

    if (name_len > MAX_TABLE_NAME_SIZE) {
        return TABLE_TOO_LONG_NAME;
    }

    for (u8 i = 0; i < columns_count; ++i) {
        if (strlen(columns[i].name) > MAX_TABLE_COLUMN_NAME_SIZE) {
            return TABLE_TOO_LONG_COLUMN_NAME;
        }
    }

    return TABLE_OK;
}

static void table_deserialize_name(char** name, BufReader* reader)
{
    u8 name_len;
    buf_reader_read(reader, &name_len, 1);

    *name = malloc(name_len + 1);
    (*name)[name_len] = 0;
    buf_reader_read(reader, *name, name_len);
}

static bool type_id_is_nullable(u8 type_id) { return type_id >= 16; }

static void table_deserialize_data_types(Column* columns, u8 columns_count,
                                         BufReader* reader)
{
    for (u8 i = 0; i < columns_count; ++i) {
        DataType type = { .max_size = 0 };
        u8 type_id_buf;
        buf_reader_read(reader, &type_id_buf, 1);

        type.nullable = type_id_is_nullable(type_id_buf);
        if (type.nullable) {
            type_id_buf -= 16;
        }

        type.id = type_id_buf;

        if (type_id_buf == CHAR || type_id_buf == VARCHAR) {
            u16 max_size_buf;
            buf_reader_read(reader, &max_size_buf, 2);
            type.max_size = max_size_buf;
        }

        columns[i].type = type;
    }
}

static void table_deserialize_column_names(Column* columns, u8 columns_count,
                                           BufReader* reader)
{

    for (u8 i = 0; i < columns_count; ++i) {
        u8 name_len;
        buf_reader_read(reader, &name_len, 1);

        columns[i].name = malloc(name_len + 1);
        columns[i].name[name_len] = 0;
        buf_reader_read(reader, columns[i].name, name_len);
    }
}

static void table_deserialize_columns(Column** columns, u8 columns_count,
                                      BufReader* reader)
{
    *columns = malloc((sizeof **(columns)) * columns_count);

    table_deserialize_data_types(*columns, columns_count, reader);
    table_deserialize_column_names(*columns, columns_count, reader);
}

u64 table_serialized_size(const Table* t)
{
    u64 size = 12; // Records addresses size.
    size += 8; // Records count size.

    size += 1; // Name length size.
    size += strlen(t->name);

    size += 1; // Column counts size.

    for (u8 i = 0; i < t->columns_count; ++i) {
        size += DATATYPE_SIZE(t->columns[i].type);
    }

    size += t->columns_count; // For column names size.

    for (u8 i = 0; i < t->columns_count; ++i) {
        size += strlen(t->columns[i].name);
    }

    return size;
}

static void table_serialize_name(const char* name, BufWriter* writer)
{

    u8 name_len = (u8)strlen(name);
    buf_writer_write(writer, &name_len, 1);
    buf_writer_write(writer, name, name_len);
}

static void table_serialize_column_types(const Column* columns,
                                         u8 columns_count, BufWriter* writer)
{
    for (u8 i = 0; i < columns_count; ++i) {
        u8 type = (u8)columns[i].type.id;

        if (columns[i].type.nullable) {
            type += 16;
        }

        buf_writer_write(writer, &type, 1);

        if (columns[i].type.id == CHAR || columns[i].type.id == VARCHAR) {
            buf_writer_write(writer, &(columns[i].type.max_size), 2);
        }
    }
}

static void table_serialize_column_names(const Column* columns,
                                         u8 columns_count, BufWriter* writer)
{
    for (u8 i = 0; i < columns_count; ++i) {
        u8 name_len = (u8)strlen(columns[i].name);

        buf_writer_write(writer, &name_len, 1);
        buf_writer_write(writer, columns[i].name, name_len);
    }
}

static void table_serialize_columns(const Column* columns, u8 columns_count,
                                    BufWriter* writer)
{
    table_serialize_column_types(columns, columns_count, writer);
    table_serialize_column_names(columns, columns_count, writer);
}

ListBlock list_block_from_table(const Table* table)
{
    SerializedTable serialized = table_serialize(table);
    ListBlock block = list_block_new(LIST_BLOCK_TYPE_TABLE, serialized.bytes,
                                     serialized.size);
    serialized_table_free(&serialized);
    return block;
}

Table list_block_to_table(const ListBlock* block)
{
    return table_deserialize(block->payload, block->header.payload_size);
}
