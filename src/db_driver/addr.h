#pragma once

#include <stdbool.h>
#include <types.h>

#define NullAddr (addr_new(0, 0, 0))

typedef struct {
  u64 page_size;
  u32 page_number;
  u16 offset;
} Addr;

Addr addr_new(u32 page_number, u16 offset, u64 page_size);

// Calculate absolute offset from file start.
u64 addr_offset(Addr addr);

bool addr_cmp(Addr addr1, Addr addr2);
