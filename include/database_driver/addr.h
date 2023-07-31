#pragma once

#include "nclib.h"

typedef struct {
    i32 page_id; // Page id, starts from zero. -1 value mean NULL_ADDR
    i16 offset; // Offset inside current page, starts from zero. -1 value
                // mean NULL_ADDR
} Addr;

#define NULL_ADDR ((Addr) { .page_id = -1, .offset = -1 })

bool addr_eq(Addr, Addr);
bool addr_is_null(Addr);
