#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../utils/buf_reader.h"
#include "../utils/buf_writer.h"
#include "list_block.h"
#include "page.h"

ListBlock list_block_new(ListBlockType type, u8 *payload, u64 payload_size) {
    ListBlock block = {
        .type = type,
        .is_overflow = (payload_size > PAGE_PAYLOAD_SIZE) ? true : false,
        .payload = (u8 *)malloc(payload_size),
        .payload_size = payload_size,
        .next = NullAddr,
    };

    memcpy(block.payload, payload, payload_size);

    return block;
}

void list_block_free(ListBlock *block) { free(block->payload); }

void list_block_write(const ListBlock *block, i32 fd, Addr addr) {
    i64 offset = addr_offset(addr);
    lseek(fd, offset, SEEK_SET);

    u8 block_header_buf[LIST_BLOCK_HEADER_SIZE];
    BufWriter writer = buf_writer_new(block_header_buf, LIST_BLOCK_HEADER_SIZE);

    buf_writer_write(&writer, &block->type, 1);
    buf_writer_write(&writer, &block->is_overflow, 1);
    buf_writer_write(&writer, &block->next, 6);
    buf_writer_write(&writer, &block->payload_size, 8);

    write(fd, buf_writer_get_buf(&writer), LIST_BLOCK_HEADER_SIZE);

    write(fd, block->payload, block->payload_size);
}

ListBlock list_block_read(i32 fd, Addr addr) {
    i64 offset = addr_offset(addr);
    lseek(fd, offset, SEEK_SET);

    u8 block_header_buf[LIST_BLOCK_HEADER_SIZE];
    read(fd, block_header_buf, LIST_BLOCK_HEADER_SIZE);

    BufReader reader = buf_reader_new(block_header_buf, LIST_BLOCK_HEADER_SIZE);

    u8 block_type_buf;
    buf_reader_read(&reader, &block_type_buf, 1);

    ListBlock block = {.type = block_type_buf};

    buf_reader_read(&reader, &block.is_overflow, 1);
    buf_reader_read(&reader, &block.next, 6);
    buf_reader_read(&reader, &block.payload_size, 8);

    block.payload = (u8 *)malloc(block.payload_size);
    read(fd, block.payload, block.payload_size);

    return block;
}
