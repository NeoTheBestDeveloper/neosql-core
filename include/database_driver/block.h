#pragma once

#include "nclib/typedefs.h"

#include "database_driver/addr.h"

#define BLOCK_HEADER_SIZE (15)
#define BLOCK_PART_HEADER_SIZE (14)

typedef struct {
    u64 payload_size;
    Addr next;
} BlockPartHeader;

typedef struct {
    u64 payload_size;
    Addr next;
    bool parted;
} BlockHeader;

typedef struct {
    BlockHeader header;
    u8* payload;
} Block;

// Read block from database by given address.
Block block_new(Addr addr, i32 fd);
void block_free(Block* block);

Addr block_find_space(const Block* block, i32 fd);
void block_write(const Block* block, Addr addr, i32 fd);
