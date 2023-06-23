#pragma once

#include <types.h>

#include "../utils/buf_writer.h"
#include "db_header.h"

#define PAGE_SIZE (4096)
#define PAGE_HEADER_SIZE (32)
#define PAGE_PAYLOAD_SIZE (PAGE_SIZE - PAGE_HEADER_SIZE)

typedef struct {
    u8 *payload;
    u16 page_id;
    u16 free_space;
    u16 first_free_byte;
} Page;

Page page_new(u16 page_id);
void page_free(Page *page);

Page page_read(u16 page_id, i32 fd);
void page_write(const Page *page, i32 fd);
