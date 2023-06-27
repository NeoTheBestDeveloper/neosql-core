#pragma once

#include <stdbool.h>
#include <stdint.h>

#define NULL_ADDR ((Addr){.page_id = 0, .offset = 0})

typedef struct {
    int32_t page_id; // Starts from zero.
    int16_t offset;  // Offset inside page payload, not inside all page.
} Addr;

// Calculate absolute addres offset from file start.
int64_t addr_offset(Addr);

// Return true if addresses is the same.
bool addr_cmp(Addr, Addr);

bool is_null(Addr);
