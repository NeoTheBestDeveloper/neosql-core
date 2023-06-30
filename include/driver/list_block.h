#pragma once

#include <stdint.h>

#include "addr.h"

#define LIST_BLOCK_HEADER_SIZE (16)

typedef enum {
    LIST_BLOCK_TYPE_TABLE = 0,
    LIST_BLOCK_TYPE_RECORD = 1,
} ListBlockType;

typedef struct {
    uint64_t payload_size;
    Addr next;
    ListBlockType type; // At file 1 byte signed number.
    bool is_overflow;
} ListBlockHeader;

typedef struct {
    ListBlockHeader header;
    uint8_t* payload;
} ListBlock;

// Create new block, copy payload from "payload" argument.
ListBlock list_block_new(ListBlockType type, uint8_t const* payload,
                         uint64_t payload_size);
void list_block_free(ListBlock*);
