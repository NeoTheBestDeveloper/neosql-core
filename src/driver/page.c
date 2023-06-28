#include <stdlib.h>
#include <unistd.h>

#include "driver/page.h"
#include "utils/buf_reader.h"
#include "utils/buf_writer.h"

// Calculate offset inside entire file.
static int64_t calc_page_payload_offset(int16_t page_id) {
    return page_id * DEFAULT_PAGE_SIZE + HEADER_SIZE;
}

Page page_new(int16_t page_id) {
    return (Page){
        .first_free_byte = 0,
        .free_space = PAGE_PAYLOAD_SIZE,
        .page_id = page_id,
        .payload = calloc(PAGE_PAYLOAD_SIZE, 1),
    };
}

Page page_read(int16_t page_id, int32_t fd) {
    Page page = {
        .page_id = page_id,
    };

    int64_t offset = calc_page_payload_offset(page_id);
    lseek(fd, offset, SEEK_SET);

    uint8_t page_header[PAGE_HEADER_SIZE] = {0};
    read(fd, page_header, PAGE_HEADER_SIZE);

    page.free_space = *(int16_t *)page_header;
    page.first_free_byte = *(int16_t *)(page_header + 2);

    page.payload = malloc(PAGE_PAYLOAD_SIZE);
    read(fd, page.payload, PAGE_PAYLOAD_SIZE);

    return page;
}

void page_write(Page const *page, int32_t fd) {
    int64_t offset = calc_page_payload_offset(page->page_id);

    uint8_t page_header[PAGE_HEADER_SIZE] = {0};
    BufWriter header_writer = buf_writer_new(page_header, PAGE_HEADER_SIZE);

    buf_writer_write(&header_writer, &(page->free_space), 2);
    buf_writer_write(&header_writer, &(page->first_free_byte), 2);

    lseek(fd, offset, SEEK_SET);

    uint8_t const *header = buf_writer_get_buf(header_writer);

    write(fd, header, PAGE_HEADER_SIZE);
    write(fd, page->payload, PAGE_PAYLOAD_SIZE);
}

void page_free(Page *page) { free(page->payload); }
