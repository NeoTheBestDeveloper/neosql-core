#pragma once

#include "nclib.h"

#define PAGE_HEADER_SIZE (32)
#define PAGE_HEADER_RESERVED_SIZE (PAGE_HEADER_SIZE - 2)

#define PAGE_SIZE (4096)
#define PAGE_PAYLOAD_SIZE (PAGE_SIZE - PAGE_HEADER_SIZE)

typedef struct {
    u8* payload;
    i32 id;
    u16 free_space;
} Page;

Page page_new(i32 page_id, i32 fd);
Page page_new_zero(i32 page_id);
void page_write(const Page*, i32 fd);

void page_free(Page*);

void seek_to_page(i32 fd, i32 page_id);
