#pragma once

#include <stdbool.h>
#include <types.h>

#define NullAddr (addr_new(0, 0))

typedef struct {
    u32 page_id;
    u16 offset;
} Addr;

Addr addr_new(u32 page_id, u16 offset);

// Calculate absolute addres offset from file start.
i64 addr_offset(Addr addr);

bool addr_cmp(Addr addr1, Addr addr2);
