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

typedef enum {
    PAGE_ok = 0,
    PAGE_overflow = 1,
    PAGE_not_enough_space = 2,
} PageResultStatus;

typedef struct {
    int16_t payload_processed;
    PageResultStatus status;
} PageResult;

// Create page with "DEFAULT_PAGE_SIZE" and fill payload with zeroes.
Page page_new(int16_t page_id);
void page_free(Page *);

// Some base io stuff for pages.
Page page_read(int16_t page_id, int32_t fd);
void page_write(Page, int32_t fd);
