#pragma once

#include "nclib/typedefs.h"

#include "database_driver/addr.h"

typedef struct {
    u64 payload_size;
    Addr next;
    bool parted;
} BlockHeader;

typedef struct {
    u64 payload_size;
    Addr next;
} BlockPartHeader;

typedef struct {
    BlockPartHeader header;
    u8* payload;
} BlockPart;

typedef struct {
    BlockHeader header;
    u8* payload;
} Block;

// Read block from database by given address.
Block block_new(Addr addr);
void block_free(Block* block);

void block_write(const Block* block, Addr addr);
