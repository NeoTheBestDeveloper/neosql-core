#include <stdlib.h>
#include <string.h>

#include "criterion/assert.h"
#include "criterion/criterion.h"
#include "criterion/new/assert.h"

#include "driver/addr.h"
#include "driver/list_block.h"
#include "table.h"
#include "utils/buf_reader.h"
#include "utils/buf_writer.h"

Test(TestBlock, test_list_block_from_table)
{
    Column c1 = {
        .name = strdup("col1"),
        { .id = MICROINT, .nullable = false, .max_size = 0 },
    };

    Column c2 = {
        .name = strdup("col2"),
        { .id = CHAR, .nullable = true, .max_size = 4980 },
    };

    Column columns[] = { c1, c2 };

    TableResult res = table_new("Aboba table", columns, 2);

    if (res.status != TABLE_OK) {
        cr_assert(false);
    }

    ListBlock block = list_block_from_table(&(res.table));

    cr_assert(eq(u8, block.header.is_overflow, false));
    cr_assert(eq(u32, block.header.type, LIST_BLOCK_TYPE_TABLE));
    cr_assert(addr_cmp(block.header.next, NULL_ADDR));
    cr_assert(eq(u64, block.header.payload_size,
                 table_serialized_size(&(res.table))));

    BufReader reader
        = buf_reader_new(block.payload, block.header.payload_size);

    Addr first_record_buf, last_record_buf;

    buf_reader_read(&reader, &first_record_buf, 6);
    buf_reader_read(&reader, &last_record_buf, 6);

    cr_assert(addr_cmp(first_record_buf, NULL_ADDR));
    cr_assert(addr_cmp(last_record_buf, NULL_ADDR));

    u8 columns_count;

    buf_reader_read(&reader, &columns_count, 1);
    cr_assert(eq(u8, columns_count, res.table.columns_count));

    u64 records_count_buf;
    buf_reader_read(&reader, &records_count_buf, 8);

    cr_assert(eq(u64, records_count_buf, 0));

    u8 name_len_buf;
    buf_reader_read(&reader, &name_len_buf, 1);
    cr_assert(eq(u64, name_len_buf, strlen(res.table.name)));

    char name[1000];
    buf_reader_read(&reader, name, name_len_buf);
    cr_assert(eq(str, name, res.table.name));

    u8 type_id1_buf, type_id2_buf;

    buf_reader_read(&reader, &type_id1_buf, 1);
    cr_assert(eq(u32, type_id1_buf, MICROINT));

    buf_reader_read(&reader, &type_id2_buf, 1);
    cr_assert(eq(u32, type_id2_buf, CHAR + 16));

    u16 max_size_buf;
    buf_reader_read(&reader, &max_size_buf, 2);
    cr_assert(eq(u16, max_size_buf, c2.type.max_size));

    u8 column_name_len1, column_name_len2;
    char name1[1000];
    char name2[1000];

    buf_reader_read(&reader, &column_name_len1, 1);
    cr_assert(eq(u8, column_name_len1, strlen(c1.name)));

    buf_reader_read(&reader, name1, column_name_len1);
    cr_assert(eq(str, name1, c1.name));

    buf_reader_read(&reader, &column_name_len2, 1);
    cr_assert(eq(u8, column_name_len2, strlen(c2.name)));

    buf_reader_read(&reader, name2, column_name_len2);
    cr_assert(eq(str, name2, c2.name));

    free(c1.name);
    free(c2.name);

    table_free(&(res.table));
    list_block_free(&block);
}

Test(TestBlock, test_list_block_to_table)
{
    u8 payload[1000] = { 0 };
    ListBlock block = { 
        .payload = payload,
        .header = {
                        .is_overflow = false,
                        .next = NULL_ADDR,
                        .payload_size = 0, 
                        .type = LIST_BLOCK_TYPE_TABLE,
                    }, 
    };

    BufWriter writer = buf_writer_new(payload, 1000);

    Addr first_record = { .offset = 10, .page_id = 1 };
    Addr last_record = { .offset = 109, .page_id = 129 };

    buf_writer_write(&writer, &first_record, 6);
    buf_writer_write(&writer, &last_record, 6);

    u8 columns_count = 2;

    buf_writer_write(&writer, &columns_count, 1);

    u64 records_count = 101;
    buf_writer_write(&writer, &records_count, 8);

    const char* name = "COOL Table";
    u8 name_len = (u8)strlen(name);

    buf_writer_write(&writer, &name_len, 1);
    buf_writer_write(&writer, name, name_len);

    DataType type1 = { .id = TEXT, .nullable = true, .max_size = 0 };
    DataType type2 = { .id = VARCHAR, .nullable = true, .max_size = 1024 };

    u8 type_id1 = TEXT + 16;
    u8 type_id2 = VARCHAR + 16;

    buf_writer_write(&writer, &type_id1, 1);
    buf_writer_write(&writer, &type_id2, 1);
    buf_writer_write(&writer, &(type2.max_size), 2);

    const char* c1_name = "fgfds";
    const char* c2_name = "gfdgd";

    u8 c1_name_len = (u8)strlen(c1_name);
    u8 c2_name_len = (u8)strlen(c2_name);

    buf_writer_write(&writer, &c1_name_len, 1);
    buf_writer_write(&writer, c1_name, c1_name_len);

    buf_writer_write(&writer, &c2_name_len, 1);
    buf_writer_write(&writer, c2_name, c2_name_len);

    block.payload = writer.buf;
    block.header.payload_size = writer.buf_offset;

    Table table = list_block_to_table(&block);

    cr_assert(eq(u8, table.columns_count, columns_count));
    cr_assert(eq(u64, table.records_count, records_count));
    cr_assert(eq(str, table.name, name));

    cr_assert(addr_cmp(table.first_record, first_record));
    cr_assert(addr_cmp(table.last_record, table.last_record));

    cr_assert(eq(str, table.columns[0].name, c1_name));
    cr_assert(eq(str, table.columns[1].name, c2_name));

    cr_assert(eq(u32, table.columns[0].type.id, type1.id));
    cr_assert(eq(u32, table.columns[1].type.id, type2.id));

    cr_assert(eq(u8, table.columns[0].type.nullable, type1.nullable));
    cr_assert(eq(u8, table.columns[1].type.nullable, type2.nullable));

    cr_assert(eq(u16, table.columns[0].type.max_size, type1.max_size));
    cr_assert(eq(u16, table.columns[1].type.max_size, type2.max_size));

    table_free(&table);
}
