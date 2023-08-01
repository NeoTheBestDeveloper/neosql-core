#include "utils/stream_ext.h"
#include "nclib/stream.h"

Addr stream_read_addr(Stream* stream)
{
    Addr addr;

    addr.page_id = stream_read_i32(stream);
    addr.offset = stream_read_i16(stream);

    return addr;
}

void stream_write_addr(Stream* stream, Addr addr)
{
    stream_write_i32(stream, addr.page_id);
    stream_write_i16(stream, addr.offset);
}

Header stream_read_header(Stream* stream)
{
    stream_seek(stream, HEADER_MAGIC_SIZE, STREAM_START);

    Header header;

    header.pages_count = stream_read_u32(stream);
    header.first_table = stream_read_addr(stream);
    header.last_table = stream_read_addr(stream);
    header.cached_pages_count = stream_read_u32(stream);

    return header;
}

void stream_write_header(Stream* stream, const Header* header)
{
    stream_write_bytes(stream, (const u8*)HEADER_MAGIC, HEADER_MAGIC_SIZE);
    stream_write_u32(stream, header->pages_count);
    stream_write_addr(stream, header->first_table);
    stream_write_addr(stream, header->last_table);
    stream_write_u32(stream, header->cached_pages_count);

    u8 reserved[HEADER_RESERVED] = { 0 };
    stream_write_bytes(stream, reserved, HEADER_RESERVED);
}

Page stream_read_page(Stream* stream)
{
    Page page;

    page.free_space = stream_read_u16(stream);
    page.payload = stream->buf + PAGE_HEADER_SIZE;

    return page;
}

void stream_write_page(Stream* stream, const Page* page)
{
    stream_write_u16(stream, page->free_space);
    stream_seek(stream, PAGE_HEADER_SIZE, STREAM_START);
    stream_write_bytes(stream, page->payload, PAGE_PAYLOAD_SIZE);
}
