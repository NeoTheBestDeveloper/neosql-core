#include <stdlib.h>
#include <string.h>

#include "driver/list_block.h"
#include "driver/page.h"
#include "utils/buf_reader.h"
#include "utils/buf_writer.h"

ListBlock list_block_new(ListBlockType type, uint8_t const *payload,
                         uint64_t payload_size) {
    ListBlock block = {
        .type = type,
        .is_overflow = (payload_size > PAGE_PAYLOAD_SIZE),
        .payload = malloc(payload_size),
        .payload_size = payload_size,
        .next = NULL_ADDR,
    };

    memcpy(block.payload, payload, payload_size);

    return block;
}

void list_block_free(ListBlock *block) { free(block->payload); }
