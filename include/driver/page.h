#pragma once

#include <stdint.h>

#include "defaults.h"
#include "header.h"
#include "list_block.h"

#define PAGE_HEADER_SIZE (32)
#define PAGE_HEADER_RESERVED_SIZE (PAGE_HEADER_SIZE - 4)
#define PAGE_PAYLOAD_SIZE (DEFAULT_PAGE_SIZE - PAGE_HEADER_SIZE)

typedef struct {
    uint8_t *payload;
    int16_t page_id;
    int16_t free_space;
    int16_t first_free_byte; // Offset inside payload, not all page.
} Page;

// Create page with "DEFAULT_PAGE_SIZE" and fill payload with zeroes.
Page page_new(int16_t page_id);
void page_free(Page *);

// Some base io stuff for pages.
Page page_read(int16_t page_id, int32_t fd);
void page_write(Page const *, int32_t fd);
