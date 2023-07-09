#include <stdlib.h>
#include <string.h>

#include "driver/list_block.h"
#include "driver/page.h"
#include "utils/buf_reader.h"
#include "utils/buf_writer.h"
#include "utils/mem.h"

ListBlock list_block_new(ListBlockType type, u8 const* payload,
                         u64 payload_size)
{
    ListBlock block = {
        .header =
            {
                .type = type,
                .is_overflow = (payload_size > PAGE_PAYLOAD_SIZE),
                .payload_size = payload_size,
                .next = NULL_ADDR,
            },
        .payload = copy_mem(payload, payload_size),
    };

    return block;
}

void list_block_free(ListBlock* block) { free(block->payload); }
