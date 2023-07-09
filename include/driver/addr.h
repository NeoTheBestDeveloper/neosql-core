#pragma once

#include "utils/types.h"

#define NULL_ADDR ((Addr) { .page_id = -1, .offset = -1 })

typedef struct {
    i32 page_id; // Starts from zero.
    i16 offset; // Offset inside page payload, not inside all page.
} Addr;

// Calculate absolute addres offset from file start.
int64_t addr_offset(Addr);

// Return true if addresses is the same.
bool addr_cmp(Addr, Addr);

bool is_null(Addr);
