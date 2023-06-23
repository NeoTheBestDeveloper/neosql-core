#pragma once

#include "src/db_driver/addr.h"

#define LIST_BLOCK_HEADER_SIZE (16)

typedef enum {
    TABLE_BLOCK = 0,
    LIST_BLOCK = 1,
} ListBlockType;

typedef struct {
    u64 payload_size;
    u8 *payload;
    ListBlockType type;
    Addr next;
    bool is_overflow;
} ListBlock;

ListBlock list_block_new(ListBlockType type, u8 *payload, u64 payload_size);
void list_block_free(ListBlock *block);

void list_block_write(const ListBlock *block, i32 fd, Addr addr);
ListBlock list_block_read(i32 fd, Addr addr);
