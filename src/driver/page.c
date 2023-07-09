#include <stdlib.h>
#include <unistd.h>

#include "driver/page.h"
#include "utils/buf_reader.h"
#include "utils/buf_writer.h"

// Calculate offset inside entire file.
static i64 calc_page_payload_offset(i32 page_id)
{
    return page_id * DEFAULT_PAGE_SIZE + HEADER_SIZE;
}

Page page_new(i32 page_id)
{
    return (Page) {
        .first_free_byte = 0,
        .free_space = PAGE_PAYLOAD_SIZE,
        .page_id = page_id,
        .payload = calloc(PAGE_PAYLOAD_SIZE, 1),
    };
}

Page page_read(i32 page_id, i32 fd)
{
    Page page = {
        .page_id = page_id,
    };

    i64 offset = calc_page_payload_offset(page_id);
    lseek(fd, offset, SEEK_SET);

    u8 page_header[PAGE_HEADER_SIZE] = { 0 };
    read(fd, page_header, PAGE_HEADER_SIZE);

    page.free_space = *(i16*)page_header;
    page.first_free_byte = *(i16*)(page_header + 2);

    page.payload = malloc(PAGE_PAYLOAD_SIZE);
    read(fd, page.payload, PAGE_PAYLOAD_SIZE);

    return page;
}

void page_write(const Page* page, i32 fd)
{
    i64 offset = calc_page_payload_offset(page->page_id);

    u8 page_header[PAGE_HEADER_SIZE] = { 0 };
    BufWriter header_writer = buf_writer_new(page_header, PAGE_HEADER_SIZE);

    buf_writer_write(&header_writer, &(page->free_space), 2);
    buf_writer_write(&header_writer, &(page->first_free_byte), 2);

    lseek(fd, offset, SEEK_SET);

    const u8* header = buf_writer_get_buf(header_writer);
    write(fd, header, PAGE_HEADER_SIZE);
    write(fd, page->payload, PAGE_PAYLOAD_SIZE);
}

void page_free(Page* page) { free(page->payload); }
