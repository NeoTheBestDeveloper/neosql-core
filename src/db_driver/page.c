#include <stdlib.h>
#include <unistd.h>

#include "../utils/buf_writer.h"
#include "db_header.h"

#include "page.h"

static u64 calc_page_payload_offset(u16 page_id) {
    return page_id * PAGE_SIZE + HEADER_SIZE;
}

Page page_new(u16 page_id) {
    Page page = {
        .first_free_byte = PAGE_HEADER_SIZE,
        .free_space = (u16)(PAGE_PAYLOAD_SIZE),
        .page_id = page_id,
        .payload = (u8 *)calloc(PAGE_PAYLOAD_SIZE, 1),
    };

    return page;
}

Page page_read(u16 page_id, i32 fd) {
    Page page = {
        .page_id = page_id,
    };

    u64 offset = calc_page_payload_offset(page_id);
    lseek(fd, (i64)offset, SEEK_SET);

    u8 page_header[PAGE_HEADER_SIZE] = {0};
    read(fd, page_header, PAGE_HEADER_SIZE);

    page.free_space = *(u16 *)page_header;
    page.first_free_byte = *(u16 *)(page_header + 2);

    page.payload = (u8 *)malloc(PAGE_PAYLOAD_SIZE);
    read(fd, page.payload, PAGE_PAYLOAD_SIZE);

    return page;
}

void page_write(const Page *page, i32 fd) {
    u64 offset = calc_page_payload_offset(page->page_id);

    u8 page_header[PAGE_HEADER_SIZE] = {0};
    BufWriter header_writer = buf_writer_new(page_header, PAGE_HEADER_SIZE);

    buf_writer_write(&header_writer, &page->free_space, 2);
    buf_writer_write(&header_writer, &page->first_free_byte, 2);

    lseek(fd, (i64)offset, SEEK_SET);

    const u8 *header = buf_writer_get_buf(&header_writer);

    write(fd, header, PAGE_HEADER_SIZE);
    write(fd, page->payload, PAGE_PAYLOAD_SIZE);
}

void page_free(Page *page) { free(page->payload); }
