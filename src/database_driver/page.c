#include <stdlib.h>
#include <unistd.h>

#include "nclib/stream.h"

#include "database_driver/database_defaults.h"
#include "database_driver/header.h"
#include "database_driver/page.h"

static inline i64 page_calc_offset(i32 page_id)
{
    return HEADER_SIZE + page_id * PAGE_SIZE;
}

Page page_new(i32 page_id, i32 fd)
{
    i64 offset = page_calc_offset(page_id);
    lseek(fd, offset, SEEK_SET);

    u8 serialized_page_header[PAGE_HEADER_SIZE] = { 0 };
    read(fd, serialized_page_header, PAGE_HEADER_SIZE);

    Stream page_header_stream = stream_new(
        serialized_page_header, PAGE_HEADER_SIZE, DEFAULT_DATABASE_ENDIAN);

    Page page = {
        .free_space = stream_read_u16(&page_header_stream),
        .payload = malloc(PAGE_PAYLOAD_SIZE),
        .id = page_id,
    };

    read(fd, page.payload, PAGE_PAYLOAD_SIZE);

    return page;
}

void page_write(const Page* page, i32 fd)
{
    i64 offset = page_calc_offset(page->id);
    lseek(fd, offset, SEEK_SET);

    u8 serialized_page_header[PAGE_HEADER_SIZE] = { 0 };
    Stream page_header_stream = stream_new(
        serialized_page_header, PAGE_HEADER_SIZE, DEFAULT_DATABASE_ENDIAN);
    stream_write_u16(&page_header_stream, page->free_space);

    write(fd, serialized_page_header, PAGE_HEADER_SIZE);
    write(fd, page->payload, PAGE_PAYLOAD_SIZE);
}

void page_free(Page* page) { free(page->payload); }
