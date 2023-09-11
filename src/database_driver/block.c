#include <stdlib.h>

#include "nclib/stream.h"

#include "database_driver/addr.h"
#include "database_driver/block.h"
#include "database_driver/database_defaults.h"
#include "database_driver/page.h"
#include "utils/stream_ext.h"

// PRIVATE METHODS SIGNATURE START.
static BlockHeader read_block_header(Addr addr, i32 fd);
static u8* read_block_payload(Addr addr, BlockHeader header, i32 fd);
static u8* read_parted_block_payload(Addr addr, BlockHeader header, i32 fd);
static u8* read_not_parted_block_payload(Addr addr, BlockHeader header,
                                         i32 fd);
static BlockPartHeader read_block_part(Addr next_part, Stream* payload_stream,
                                       i32 fd);
// PRIVATE METHODS SIGNATURE END.

// PUBLIC METHODS START.
Block block_new(Addr addr, i32 fd)
{
    BlockHeader header = read_block_header(addr, fd);
    u8* payload = read_block_payload(addr, header, fd);

    return (Block) { .header = header, .payload = payload };
}

void block_free(Block* block) { free(block->payload); }

Addr block_append(const Block* block, i32 fd) { }
// PUBLIC METHODS END.

static BlockHeader read_block_header(Addr addr, i32 fd)
{
    Page page = page_new(addr.page_id, fd);

    Stream stream
        = stream_new(page.payload, PAGE_PAYLOAD_SIZE, DEFAULT_DATABASE_ENDIAN);
    stream_seek(&stream, addr.offset, STREAM_START);

    BlockHeader header = stream_read_block_header(&stream);

    page_free(&page);

    return header;
}

static u8* read_block_payload(Addr addr, BlockHeader header, i32 fd)
{
    if (header.parted) {
        return read_parted_block_payload(addr, header, fd);
    }
    return read_not_parted_block_payload(addr, header, fd);
}

static u8* read_parted_block_payload(Addr addr, BlockHeader header, i32 fd)
{
    u8* payload = malloc(header.payload_size);
    Stream payload_stream
        = stream_new(payload, header.payload_size, DEFAULT_DATABASE_ENDIAN);

    Addr next_part = {
        .page_id = addr.page_id,
        .offset = addr.offset + BLOCK_HEADER_SIZE,
    };

    while (!addr_eq(next_part, NULL_ADDR)) {
        BlockPartHeader part_header
            = read_block_part(next_part, &payload_stream, fd);
        next_part = part_header.next;
    }

    return payload;
}

static u8* read_not_parted_block_payload(Addr addr, BlockHeader header, i32 fd)
{
    Page page = page_new(addr.page_id, fd);

    Stream stream
        = stream_new(page.payload, PAGE_PAYLOAD_SIZE, DEFAULT_DATABASE_ENDIAN);
    stream_seek(&stream, addr.offset + BLOCK_HEADER_SIZE, STREAM_START);

    u8* payload = malloc(header.payload_size);

    stream_read_bytes(&stream, payload, header.payload_size);

    return payload;
}

static BlockPartHeader read_block_part(Addr next_part, Stream* payload_stream,
                                       i32 fd)
{
    Page page = page_new(next_part.page_id, fd);

    Stream page_stream
        = stream_new(page.payload, PAGE_PAYLOAD_SIZE, DEFAULT_DATABASE_ENDIAN);
    stream_seek(&page_stream, next_part.offset, STREAM_START);

    BlockPartHeader header = stream_read_block_part_header(&page_stream);
    stream_write_bytes(payload_stream, page_stream.buf + page_stream.offset,
                       header.payload_size);

    page_free(&page);

    return header;
}
