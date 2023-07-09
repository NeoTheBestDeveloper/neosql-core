#pragma once

#include "defaults.h"
#include "header.h"
#include "list_block.h"
#include "utils/types.h"

#define PAGE_HEADER_SIZE (32)
#define PAGE_HEADER_RESERVED_SIZE (PAGE_HEADER_SIZE - 4)
#define PAGE_PAYLOAD_SIZE (DEFAULT_PAGE_SIZE - PAGE_HEADER_SIZE)

typedef struct {
    u8* payload;
    i32 page_id;
    i16 free_space;
    i16 first_free_byte; // Offset inside payload, not all page.
} Page;

// Create page with "DEFAULT_PAGE_SIZE" and fill payload with zeroes.
Page page_new(i32 page_id);
void page_free(Page*);

// Some base io stuff for pages.
Page page_read(i32 page_id, i32 fd);
void page_write(Page const*, i32 fd);
