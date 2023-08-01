#include <stdlib.h>
#include <unistd.h>

#include "nclib/stream.h"

#include "database_driver/database_defaults.h"
#include "database_driver/header.h"
#include "database_driver/page.h"
#include "utils/stream_ext.h"

static inline i64 page_calc_offset(i32 page_id)
{
    return HEADER_SIZE + page_id * PAGE_SIZE;
}

Page page_new(i32 page_id, i32 fd)
{
    i64 offset = page_calc_offset(page_id);
    lseek(fd, offset, SEEK_SET);

    u8* serialized_page_buf = malloc(PAGE_SIZE);
    read(fd, serialized_page_buf, PAGE_SIZE);

    Stream stream
        = stream_new(serialized_page_buf, PAGE_SIZE, DEFAULT_DATABASE_ENDIAN);

    Page page = stream_read_page(&stream);
    page.id = page_id;

    return page;
}

void page_write(const Page* page, i32 fd)
{
    i64 offset = page_calc_offset(page->id);
    lseek(fd, offset, SEEK_SET);

    u8* serialized_page_buf = malloc(PAGE_SIZE);
    Stream stream
        = stream_new(serialized_page_buf, PAGE_SIZE, DEFAULT_DATABASE_ENDIAN);
    stream_write_page(&stream, page);

    write(fd, serialized_page_buf, PAGE_SIZE);
    free(serialized_page_buf);
}

void page_free(Page* page) { free(page->payload - PAGE_HEADER_SIZE); }
