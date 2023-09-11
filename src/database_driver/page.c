#include <stdlib.h>
#include <unistd.h>

#include "nclib/stream.h"

#include "database_driver/database_defaults.h"
#include "database_driver/header.h"
#include "database_driver/page.h"
#include "utils/stream_ext.h"

// PRIVATE METHODS SIGNATURE START.
static inline i64 calc_page_offset(i32 page_id);
static void write_page_buf(const u8* buf, i32 page_id, i32 fd);
static u8* read_page_buf(i32 page_id, i32 fd);
static Page page_deserialise(u8* buf);
static u8* page_serialise(const Page* page);
// PRIVATE METHODS SIGNATURE END.

// PUBLIC METHODS START.
Page page_new(i32 page_id, i32 fd)
{
    u8* serialized_page_buf = read_page_buf(page_id, fd);

    Page page = page_deserialise(serialized_page_buf);
    page.id = page_id;

    return page;
}

Page page_new_zero(i32 page_id)
{
    return (Page) {
        .free_space = PAGE_PAYLOAD_SIZE,
        .id = page_id,
        .payload = calloc(PAGE_PAYLOAD_SIZE, 1),
    };
}

void page_write(const Page* page, i32 fd)
{
    u8* serialized_page_buf = page_serialise(page);
    write_page_buf(serialized_page_buf, page->id, fd);
    free(serialized_page_buf);
}

void page_free(Page* page) { free(page->payload - PAGE_HEADER_SIZE); }
// PUBLIC METHODS END.

static inline i64 calc_page_offset(i32 page_id)
{
    return HEADER_SIZE + page_id * PAGE_SIZE;
}

void seek_to_page(i32 fd, i32 page_id)
{
    i64 offset = calc_page_offset(page_id);
    lseek(fd, offset, SEEK_SET);
}

static void write_page_buf(const u8* buf, i32 page_id, i32 fd)
{
    seek_to_page(fd, page_id);
    write(fd, buf, PAGE_SIZE);
}

static u8* read_page_buf(i32 page_id, i32 fd)
{
    u8* buf = malloc(PAGE_SIZE);

    seek_to_page(fd, page_id);
    read(fd, buf, PAGE_SIZE);

    return buf;
}

static Page page_deserialise(u8* buf)
{
    Stream stream = stream_new(buf, PAGE_SIZE, DEFAULT_DATABASE_ENDIAN);
    return stream_read_page(&stream);
}

// Convert page to bytes which allocated on heap.
static u8* page_serialise(const Page* page)
{
    u8* buf = malloc(PAGE_SIZE);
    Stream stream = stream_new(buf, PAGE_SIZE, DEFAULT_DATABASE_ENDIAN);
    stream_write_page(&stream, page);
    return buf;
}
