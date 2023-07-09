#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include "criterion/assert.h"
#include "criterion/internal/assert.h"
#include "table.h"
#include "utils/buf_writer.h"

Test(TestTable, test_table_validation_zero_columns)
{
    TableResult res = table_new("Abima Table", NULL, 0);

    cr_assert(eq(u32, res.status, TABLE_WITHOUT_COLUMNS_NOT_VALID));
}

Test(TestTable, test_table_validation_too_long_name)
{
    Column columns[] = {
        {
            .name = strdup("Abima"),
            .type = { .id = CHAR, .nullable = false, .max_size = 255 },
        },
    };

    char name[300] = { 0 };
    memset(name, 1, 299);
    TableResult res = table_new(name, columns, 1);
    free(columns[0].name);

    cr_assert(eq(u32, res.status, TABLE_TOO_LONG_NAME));
}

Test(TestTable, test_table_validation_too_long_column_name)
{
    Column columns[] = {
        {
            .name = calloc(300, 1),
            .type = { .id = CHAR, .nullable = false, .max_size = 255 },
        },
    };

    memset(columns[0].name, -1, 299);
    TableResult res = table_new("Aboba", columns, 1);
    free(columns[0].name);

    cr_assert(eq(u32, res.status, TABLE_TOO_LONG_COLUMN_NAME));
}

Test(TestTable, test_table_validation_ok)
{
    Column columns[] = {
        {
            .name = strdup("Abobus"),
            .type = { .id = CHAR, .nullable = false, .max_size = 255 },
        },
    };

    TableResult res = table_new("Aboba", columns, 1);
    free(columns[0].name);

    Table t = res.table;

    cr_assert(eq(u32, res.status, TABLE_OK));
    cr_assert(eq(str, t.name, "Aboba"));
    cr_assert_arr_eq(&(t.first_record), &NULL_ADDR, 6);
    cr_assert_arr_eq(&(t.last_record), &NULL_ADDR, 6);
    cr_assert(eq(str, t.columns[0].name, "Abobus"));
    cr_assert(eq(u32, t.columns[0].type.id, columns[0].type.id));
    cr_assert(eq(u16, t.columns[0].type.max_size, columns[0].type.max_size));
    cr_assert(eq(u8, t.columns[0].type.nullable, columns[0].type.nullable));
    cr_assert(eq(u64, t.records_count, 0));
    cr_assert(eq(u64, t.columns_count, 1));

    table_free(&res.table);
}

Test(TestTable, test_table_serialization)
{
    Column columns[] = {
        {
            .name = strdup("Abobus"),
            .type = { .id = CHAR, .nullable = true, .max_size = 10000 },
        },
    };

    Addr first_record = (Addr) { .page_id = 0, .offset = 100 };
    Addr last_record = (Addr) { .page_id = 3, .offset = 1027 };

    Table table = table_new("Aboba", columns, 1).table;
    free(columns[0].name);

    table.first_record = first_record;
    table.last_record = last_record;
    table.records_count = 100123;

    SerializedTable serialized = table_serialize(&table);
    u8* bytes = serialized.bytes;

    // FIXME: fix this call.
    cr_assert_arr_eq(bytes, &first_record, 6);
    cr_assert_arr_eq(bytes + 6, &last_record, 6);
    cr_assert(eq(u8, *(u8*)(bytes + 12), table.columns_count));
    cr_assert(eq(u64, *(u64*)(bytes + 13), table.records_count));
    cr_assert(eq(u8, *(u64*)(bytes + 21), strlen(table.name)));
    cr_assert_arr_eq(bytes + 22, table.name, strlen(table.name));
    cr_assert(eq(u32, *(u8*)(bytes + 27),
                 CHAR + 16 * (i32)(table.columns[0].type.nullable)));
    cr_assert(eq(u16, *(u16*)(bytes + 28), table.columns[0].type.max_size));
    cr_assert(eq(u8, *(u8*)(bytes + 30), strlen(table.columns[0].name)));
    cr_assert_arr_eq(bytes + 31, table.columns[0].name, 6);

    table_free(&table);
    serialized_table_free(&serialized);
}

Test(TestTable, test_table_deserialization)
{
    u8 buf[1000] = { 0 };
    BufWriter writer = buf_writer_new(buf, 1000);

    Addr first_record = (Addr) { .page_id = 0, .offset = 1344 };
    Addr last_record = (Addr) { .page_id = 4, .offset = 1627 };

    buf_writer_write(&writer, &first_record, 6);
    buf_writer_write(&writer, &last_record, 6);

    u8 columns_count = 3;
    buf_writer_write(&writer, &columns_count, 1);

    u64 records_count = 1324;
    buf_writer_write(&writer, &records_count, 8);

    const char* name = "SUPED Duer data.";
    u8 name_len = (u8)strlen(name);
    buf_writer_write(&writer, &name_len, 1);
    buf_writer_write(&writer, name, name_len);

    DataType type1 = { .id = BIGINT, .nullable = false, .max_size = 0 };
    DataType type2 = { .id = CHAR, .nullable = true, .max_size = 3452 };
    DataType type3 = { .id = TIMESTAMP, .nullable = true, .max_size = 0 };

    DataType types[] = { type1, type2, type3 };

    u8 type1_id = (u8)BIGINT + 16 * (u8)(type1.nullable);
    u8 type2_id = (u8)CHAR + 16 * (u8)(type2.nullable);
    u8 type3_id = (u8)TIMESTAMP + 16 * (u8)(type3.nullable);

    buf_writer_write(&writer, &type1_id, 1);
    buf_writer_write(&writer, &type2_id, 1);
    buf_writer_write(&writer, &(type2.max_size), 2);
    buf_writer_write(&writer, &type3_id, 1);

    const char* column_names[] = {
        "Column 1 b",
        "Column 2 fskljfgldsk",
        "Column 4rtg b",
    };

    for (u8 i = 0; i < columns_count; ++i) {
        u8 column_name_len = (u8)strlen(column_names[i]);
        buf_writer_write(&writer, &column_name_len, 1);
        buf_writer_write(&writer, column_names[i], column_name_len);
    }

    Table t = table_deserialize(buf, 1000);

    cr_assert_arr_eq(&(t.first_record), &first_record, 6);
    cr_assert_arr_eq(&(t.last_record), &last_record, 6);

    cr_assert(eq(u8, t.columns_count, columns_count));
    cr_assert(eq(u64, t.records_count, records_count));
    cr_assert(eq(str, t.name, name));

    for (u8 i = 0; i < columns_count; ++i) {
        cr_assert(eq(u32, t.columns[i].type.id, types[i].id));
        cr_assert(eq(u16, t.columns[i].type.max_size, types[i].max_size));
        cr_assert(eq(u8, t.columns[i].type.nullable, types[i].nullable));
        cr_assert(eq(str, t.columns[i].name, column_names[i]));
    }

    table_free(&t);
}
