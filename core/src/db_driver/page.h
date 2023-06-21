#pragma once

#include <types.h>

#include "core/src/utils/buf_writer.h"
#include "db_header.h"

#define PAGE_HEADER_SIZE (32)

typedef struct {
    BufWriter writer;
    PageSize page_size;
    u16 page_id;
    u16 free_space;
    u16 first_free_byte;
} Page;

Page page_new(PageSize page_size, u16 page_id);
void page_free(Page *page);

Page page_read(PageSize page_size, u16 page_id, i32 fd);
void page_write(const Page *page, i32 fd);

const u8 *page_get_buf(const Page *page);
