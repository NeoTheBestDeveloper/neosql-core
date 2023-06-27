#pragma once

#include <stdint.h>

#include "addr.h"

#define LIST_BLOCK_HEADER_SIZE (16)

typedef enum {
    LIST_BLOCK_TYPE_table = 0,
    LIST_BLOCK_TYPE_record = 1,
} ListBlockType;

typedef struct {
    uint8_t *payload;
    int64_t payload_size;
    Addr next;
    ListBlockType type;
    bool is_overflow;
} ListBlock;

// Create new block, copy payload from "payload" argument.
ListBlock list_block_new(ListBlockType type, uint8_t const *payload,
                         int64_t payload_size);
void list_block_free(ListBlock *);
