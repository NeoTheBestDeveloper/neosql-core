#pragma once

#include "types.h"

// Allocate at heap block of memory with selected size, copy memory to this and
// return addr of block.
void* copy_mem(const void* mem, u64 size);
