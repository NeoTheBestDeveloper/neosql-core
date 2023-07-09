#pragma once

#include "addr.h"

#define LIST_BLOCK_HEADER_SIZE (16)

typedef enum {
    LIST_BLOCK_TYPE_TABLE = 0,
    LIST_BLOCK_TYPE_RECORD = 1,
} ListBlockType;

typedef struct {
    u64 payload_size;
    Addr next;
    ListBlockType type; // At file 1 byte signed number.
    bool is_overflow;
} ListBlockHeader;

typedef struct {
    ListBlockHeader header;
    u8* payload;
} ListBlock;

// Create new block, copy payload from "payload" argument.
ListBlock list_block_new(ListBlockType type, u8 const* payload,
                         u64 payload_size);
void list_block_free(ListBlock*);
