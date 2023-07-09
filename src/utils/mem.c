#include <stdlib.h>
#include <string.h>

#include "utils/mem.h"

void* copy_mem(const void* mem, u64 size)
{
    void* mem_copy = malloc(size);
    memcpy(mem_copy, mem, size);
    return mem_copy;
}
